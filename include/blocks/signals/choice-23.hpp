#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <sstream>

namespace nrcki {
class Choice23 final : public Block {
    using Type = types::real;

    Ports<Type>* ports;

    // Внутреннее состояние для отслеживания временных задержек и рассогласований
    struct ChannelState {
        mutable types::time discordance_timer; // Таймер рассогласования
        mutable types::time clear_timer;       // Таймер сброса рассогласования
        mutable bool discordance_detected;     // Флаг обнаруженного рассогласования
        mutable bool last_state;               // Последнее состояние входа
        mutable bool change_detected;          // Флаг изменения состояния
    };

    mutable ChannelState channels[3];    // Состояния для I1, I2, I3
    mutable bool switched_to_ew;         // Флаг переключения на замещающее значение
    mutable types::time switch_timer;    // Таймер переключения
    mutable bool awaiting_second_change; // Ожидание второго изменения для EWAW=1

public:
    explicit Choice23(const Context& context) :
        Block(context),
        switched_to_ew(false), switch_timer(0), awaiting_second_change(false) {
        // Инициализация состояний каналов
        for (int i = 0; i < 3; ++i) {
            channels[i].discordance_timer    = 0;
            channels[i].clear_timer          = 0;
            channels[i].discordance_detected = false;
            channels[i].last_state           = false;
            channels[i].change_detected      = false;
        }

        // 10 входов: I1, I2, I3, EWA, EWAW, EW, QUIT, TV, TZ
        // 6 выходов: Q, AVI1, AVI2, AVI3, AVS, QEW
        ports = new Ports<Type>(10, 6);
        register_ports(ports);
    }

    void compute() const override {
        // Получаем входные сигналы
        bool I[3] = {
            *ports->inputs[0] != 0,
            *ports->inputs[1] != 0,
            *ports->inputs[2] != 0
        };

        bool EWA  = *ports->inputs[3] != 0;
        bool EWAW = *ports->inputs[4] != 0;
        bool EW   = *ports->inputs[5] != 0;
        bool QUIT = *ports->inputs[6] != 0;
        Type TV   = *ports->inputs[7]; // Разновременность срабатывания
        Type TZ   = *ports->inputs[8]; // Задержка переключения при снятии рассогласования

        // 1. Обнаружение изменений состояния входов
        for (int i = 0; i < 3; i++) {
            channels[i].change_detected = (I[i] != channels[i].last_state);
            channels[i].last_state      = I[i];
        }

        // 2. Вычисление основных логических функций по алгоритму "2 из 3"
        bool two_out_of_three = false;
        bool majority_value   = false;

        // Подсчет единиц
        int count_ones = 0;
        for (int i = 0; i < 3; i++) {
            if (I[i])
                count_ones++;
        }

        // Определение большинства
        if (count_ones >= 2) {
            two_out_of_three = true;
            majority_value   = true;
        }
        else {
            two_out_of_three = false;
            majority_value   = false;
        }

        // 3. Обнаружение рассогласований для каждого канала
        bool discordance_now[3] = {false, false, false};
        bool any_discordance    = false;

        for (int i = 0; i < 3; ++i) {
            // Канал в рассогласовании, если его значение отличается от большинства
            discordance_now[i] = (I[i] != majority_value) && (count_ones != 0 && count_ones != 3);

            if (discordance_now[i]) {
                any_discordance = true;
            }
        }

        // 4. Обновление таймеров рассогласования для каждого канала
        for (int i = 0; i < 3; ++i) {
            if (discordance_now[i]) {
                channels[i].discordance_timer += Context::dt;
                channels[i].clear_timer       = 0;

                // Если таймер превысил TV, фиксируем рассогласование
                if (channels[i].discordance_timer >= TV * Context::sec) {
                    channels[i].discordance_detected = true;
                }
            }
            else {
                channels[i].clear_timer += Context::dt;

                // Если рассогласование было обнаружено, но его нет сейчас
                if (channels[i].discordance_detected) {
                    // После времени TZ+TV сбрасываем рассогласование
                    if (channels[i].clear_timer >= (TZ + TV) * Context::sec) {
                        channels[i].discordance_detected = false;
                        channels[i].discordance_timer    = 0;
                    }
                }
                else {
                    channels[i].discordance_timer = 0;
                }
            }
        }

        // 5. Определение, сколько каналов в рассогласовании
        int discordance_count = 0;
        for (int i = 0; i < 3; ++i) {
            if (channels[i].discordance_detected) {
                discordance_count++;
            }
        }

        // 6. Обработка переключения на замещающее значение
        if (!switched_to_ew) {
            bool should_switch = false;

            // Логика переключения в зависимости от EWA и EWAW
            if (EWA && !EWAW) {
                // Переключение при рассогласовании одного канала
                should_switch = (discordance_count == 1);
            }
            else if (!EWA && EWAW) {
                // Переключение при рассогласовании двух каналов
                should_switch = (discordance_count == 2);
                // Требуется ожидание второго изменения
                if (discordance_count == 1 && any_discordance) {
                    awaiting_second_change = true;
                }
                if (awaiting_second_change && discordance_count == 2) {
                    should_switch          = true;
                    awaiting_second_change = false;
                }
            }
            else if (EWA && EWAW) {
                // Нормальный режим работы
                should_switch = false;
            }
            else {
                // EWA=0, EWAW=0 - нормальный режим
                should_switch = false;
            }

            if (should_switch) {
                switched_to_ew = true;
                switch_timer   = 0;
            }
        }
        else {
            // Уже переключены на EW - обновляем таймер
            switch_timer += Context::dt;
        }

        // 7. Обработка квитирования (QUIT)
        if (QUIT) {
            switched_to_ew         = false;
            awaiting_second_change = false;

            // Сброс всех рассогласований при квитировании
            for (int i = 0; i < 3; ++i) {
                channels[i].discordance_detected = false;
                channels[i].discordance_timer    = 0;
                channels[i].clear_timer          = 0;
            }
        }

        // 8. Формирование выходных сигналов
        // Выход Q
        if (switched_to_ew) {
            ports->outputs[0] = EW ? 1.0 : 0.0;
        }
        else {
            ports->outputs[0] = two_out_of_three ? 1.0 : 0.0;
        }

        // Сигналы рассогласования по каналам
        for (int i = 0; i < 3; ++i) {
            ports->outputs[1 + i] = channels[i].discordance_detected ? 1.0 : 0.0;
        }

        // Общий сигнал рассогласования AVS
        ports->outputs[4] = (discordance_count > 0) ? 1.0 : 0.0;

        // Сигнал переключения на замещающее значение QEW
        ports->outputs[5] = switched_to_ew ? 1.0 : 0.0;
    }

    types::string printSource() const override {
        const auto x1 = CODE_NAME_IN(ports, 0);
        const auto x2 = CODE_NAME_IN(ports, 1);

        const auto y1 = CODE_NAME_OUT(ports, 0);
        const auto y2 = CODE_NAME_OUT(ports, 1);
        const auto y3 = CODE_NAME_OUT(ports, 2);

        std::stringstream line;
        line << y1 << " = (" << x1 << " != 0) || (" << x2 << " != 0);\n";
        line << y2 << " = (" << x1 << " != 0) && (" << x2 << " != 0);\n";
        line << y3 << " = (" << x1 << " != 0) != (" << x2 << " != 0);";
        return line.str();
    }
};
}