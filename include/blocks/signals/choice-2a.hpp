#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <algorithm>
#include <sstream>

namespace nrcki {
class Choice2A final : public Block {
    using Type = types::real;

    Ports<Type>* ports;

    enum Mode {
        MIN,
        MAX,
        MID,
        CHOICE
    };

    // Параметры блока
    Type VZAB;      // Постоянная времени для разности
    Type GWAB;      // Предел разности
    Type VZSFU;     // Постоянная времени для сглаживания выхода
    Type GWSFU;     // Предел скачка для отключения сглаживания
    Type VW1;       // X1 основной
    Type VW2;       // X2 основной
    Type SAB;       // Блокировка ABW
    Type MIMA;      // Режим min/max
    Type MIT;       // Режим среднего
    Type SSFU;      // Отключение сглаживания
    types::time TW; // Время задержки недостоверности

    // Внутреннее состояние
    mutable Type prev_Y;        // Предыдущее значение Y для сглаживания
    mutable Type prev_diff;     // Предыдущее значение разности для ABW
    mutable types::time timer1; // Таймер для FG1
    mutable types::time timer2; // Таймер для FG2
    mutable bool initialized;   // Флаг инициализации

    Mode getMode() const {
        if (VW1 != 0 || VW2 != 0)
            return CHOICE;
        if (MIT != 0)
            return MID;
        return MIMA != 0 ? MAX : MIN;
    }

    // Функция сглаживания (инерционное звено первого порядка)
    Type smooth(Type current, Type prev, Type time_constant, Type dt) const {
        if (time_constant <= 0)
            return current;
        Type alpha = dt / (time_constant + dt);
        return prev + alpha * (current - prev);
    }

public:
    explicit Choice2A(const Context& context,
                      Type VZAB = 3, Type GWAB = 4, Type VZSFU = 5, Type GWSFU = 2.5,
                      Type VW1  = 0, Type VW2  = 0, Type SAB   = 0, Type MIMA  = 0,
                      Type MIT  = 0, Type SSFU = 0, Type TW    = 3) :
        Block(context),
        VZAB(VZAB), GWAB(GWAB), VZSFU(VZSFU), GWSFU(GWSFU),
        VW1(VW1), VW2(VW2), SAB(SAB), MIMA(MIMA), MIT(MIT), SSFU(SSFU), TW(TW * Context::sec),
        prev_Y(0), prev_diff(0), timer1(0), timer2(0), initialized(false) {
        ports = new Ports<Type>(4, 7);
        register_ports(ports);
    }

    void compute() const override {
        // Получаем входные сигналы
        Type X1  = *ports->inputs[0];
        Type X2  = *ports->inputs[1];
        Type FG1 = *ports->inputs[2];
        Type FG2 = *ports->inputs[3];

        // Обработка недостоверных сигналов с задержкой
        bool fg1_ok = (FG1 != 0);
        bool fg2_ok = (FG2 != 0);

        // Таймеры для недостоверных сигналов
        if (!fg1_ok)
            timer1 += Context::dt;
        else
            timer1 = 0;

        if (!fg2_ok)
            timer2 += Context::dt;
        else
            timer2 = 0;

        // Сигнал считается недостоверным после задержки TW
        bool fg1_valid = (timer1 < TW);
        bool fg2_valid = (timer2 < TW);

        // Выходные сигналы достоверности
        ports->outputs[1] = (!fg1_valid || !fg2_valid) ? 1.0 : 0.0; // S1V2
        ports->outputs[2] = (!fg1_valid && !fg2_valid) ? 1.0 : 0.0; // S2V2
        ports->outputs[6] = (fg1_valid || fg2_valid) ? 1.0 : 0.0;   // FG

        // Вычисление выходного значения
        Type raw_value = 0;
        Mode mode      = getMode();

        if (!fg1_valid && !fg2_valid) // Оба сигнала недостоверны
            raw_value = 0;
        else if (!fg1_valid) // Только X1 недостоверен
            raw_value = X2;
        else if (!fg2_valid) // Только X2 недостоверен
            raw_value = X1;
        else // Оба сигнала достоверны
            switch (mode) {
                case MIN:
                    raw_value = std::min(X1, X2);
                    break;
                case MAX:
                    raw_value = std::max(X1, X2);
                    break;
                case MID:
                    raw_value = (X1 + X2) / 2.0;
                    break;
                case CHOICE:
                    raw_value = VW1 != 0 ? X1 : X2;
                    break;
            }

        // Сглаживание выходного сигнала
        if (!initialized) {
            prev_Y      = raw_value;
            initialized = true;
        }

        Type output_Y = raw_value;

        if (SSFU == 0) {
            // Сглаживание разрешено
            Type jump = std::abs(raw_value - prev_Y);
            if (jump > GWSFU) {
                // Применяем инерционное звено только если скачок превышает предел
                output_Y = smooth(raw_value, prev_Y, VZSFU, Context::dt_sec);
            }
        }

        prev_Y            = output_Y;
        ports->outputs[0] = output_Y;

        // Формирование сигнала отклонения ABW
        Type diff          = std::abs(X2 - X1);
        Type smoothed_diff = smooth(diff, prev_diff, VZAB, Context::dt_sec);
        prev_diff          = smoothed_diff;

        if (SAB == 0) {
            // Формирование ABW разрешено
            ports->outputs[3] = (smoothed_diff > GWAB) ? 1.0 : 0.0;
        }
        else {
            ports->outputs[3] = 0.0;
        }

        // Формирование сигналов выбора основного входа
        if (mode == CHOICE) {
            if (!fg1_valid && fg2_valid) {
                ports->outputs[4] = 0.0; // X1F
                ports->outputs[5] = 1.0; // X2F
            }
            else if (fg1_valid && !fg2_valid) {
                ports->outputs[4] = 1.0; // X1F
                ports->outputs[5] = 0.0; // X2F
            }
            else if (fg1_valid && fg2_valid) {
                if (VW1 != 0) {
                    ports->outputs[4] = 1.0; // X1F
                    ports->outputs[5] = 0.0; // X2F
                }
                else {
                    ports->outputs[4] = 0.0; // X1F
                    ports->outputs[5] = 1.0; // X2F
                }
            }
            else {
                ports->outputs[4] = 0.0; // X1F
                ports->outputs[5] = 0.0; // X2F
            }
        }
        else {
            // В других режимах X1F и X2F не активны
            ports->outputs[4] = 0.0;
            ports->outputs[5] = 0.0;
        }
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