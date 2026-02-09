#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <algorithm>
#include <sstream>

namespace nrcki {
class ChoiceMedian3A final : public Block {
    using Type = types::real;

    Ports<Type>* ports;

    // Внутреннее состояние для задержек рассогласования
    mutable types::time timer1; // Таймер для X1
    mutable types::time timer2; // Таймер для X2
    mutable types::time timer3; // Таймер для X3

    mutable bool st1_active; // Флаг активного состояния ST1
    mutable bool st2_active; // Флаг активного состояния ST2
    mutable bool st3_active; // Флаг активного состояния ST3

    mutable Type last_Y; // Последнее значение Y для стабилизации

    // Структура для хранения значений с индексами
    struct ValueWithIndex {
        Type value;
        int index; // 0: X1, 1: X2, 2: X3
        bool operator<(const ValueWithIndex& other) const {
            return value < other.value;
        }
    };

    // Функция гистерезиса с верхним и нижним порогами
    bool checkDeviation(Type diff, Type HM, Type HY) const {
        // HM - допустимое рассогласование, HY - гистерезис
        static Type last_state[3] = {0, 0, 0};

        // Если разница превышает HM + HY/2, то точно есть рассогласование
        if (diff > HM + HY / 2)
            return true;
        // Если разница меньше HM - HY/2, то точно нет рассогласования
        if (diff < HM - HY / 2)
            return false;
        // В зоне гистерезиса сохраняем предыдущее состояние
        return false; // По умолчанию - нет рассогласования
    }

public:
    explicit ChoiceMedian3A(const Context& context) :
        Block(context),
        timer1(0), timer2(0), timer3(0),
        st1_active(false), st2_active(false), st3_active(false),
        last_Y(0) {
        ports = new Ports<Type>(15, 4);
        register_ports(ports);
    }

    void compute() const override {
        // Получаем входные сигналы
        Type X1 = *ports->inputs[0];
        Type X2 = *ports->inputs[1];
        Type X3 = *ports->inputs[2];

        Type ERW1 = *ports->inputs[3];
        Type ERW2 = *ports->inputs[4];
        Type ERW3 = *ports->inputs[5];

        Type HM = *ports->inputs[6]; // Допустимое рассогласование
        Type HY = *ports->inputs[7]; // Величина гистерезиса
        Type TV = *ports->inputs[8]; // Время задержки

        Type FG1 = *ports->inputs[9];
        Type FG2 = *ports->inputs[10];
        Type FG3 = *ports->inputs[11];

        Type MU  = *ports->inputs[12]; // Подавление сигналов рассогласования
        Type MIN = *ports->inputs[13]; // Выбор минимального
        Type MAX = *ports->inputs[14]; // Выбор максимального

        // 1. Определение используемых значений (с учетом достоверности и замещений)
        Type val1 = (FG1 != 0) ? X1 : ERW1;
        Type val2 = (FG2 != 0) ? X2 : ERW2;
        Type val3 = (FG3 != 0) ? X3 : ERW3;

        // 2. Нахождение медианного значения
        ValueWithIndex values[3] = {
            {val1, 0},
            {val2, 1},
            {val3, 2}
        };

        // Сортировка для нахождения медианы
        ValueWithIndex sorted[3] = {values[0], values[1], values[2]};
        std::sort(sorted, sorted + 3);

        Type median_val = sorted[1].value; // Медианное значение
        int median_idx  = sorted[1].index; // Индекс медианного значения

        // 3. Определение рассогласований с учетом гистерезиса и временной задержки
        Type diff1 = std::abs(val1 - median_val);
        Type diff2 = std::abs(val2 - median_val);
        Type diff3 = std::abs(val3 - median_val);

        // Проверка превышения допустимого рассогласования
        bool exceed1 = (diff1 > HM);
        bool exceed2 = (diff2 > HM);
        bool exceed3 = (diff3 > HM);

        // Обновление таймеров для каждого канала
        if (exceed1 && FG1 != 0) {
            timer1 += Context::dt;
        }
        else {
            timer1     = 0;
            st1_active = false;
        }

        if (exceed2 && FG2 != 0) {
            timer2 += Context::dt;
        }
        else {
            timer2     = 0;
            st2_active = false;
        }

        if (exceed3 && FG3 != 0) {
            timer3 += Context::dt;
        }
        else {
            timer3     = 0;
            st3_active = false;
        }

        // Формирование сигналов ST с учетом временной задержки TV
        bool ST1_val = false;
        bool ST2_val = false;
        bool ST3_val = false;

        if (MU == 0) {
            // Подавление не активно
            if (timer1 >= TV * Context::sec && FG1 != 0) {
                ST1_val    = true;
                st1_active = true;
            }
            if (timer2 >= TV * Context::sec && FG2 != 0) {
                ST2_val    = true;
                st2_active = true;
            }
            if (timer3 >= TV * Context::sec && FG3 != 0) {
                ST3_val    = true;
                st3_active = true;
            }
        }

        // 4. Определение нарушений (сигналы ST или недостоверность FG)
        int bad_count = 0;
        if (ST1_val || FG1 == 0)
            bad_count++;
        if (ST2_val || FG2 == 0)
            bad_count++;
        if (ST3_val || FG3 == 0)
            bad_count++;

        // 5. Формирование выходного значения Y
        Type output_Y = median_val; // По умолчанию - медиана

        if (bad_count >= 2) {
            // Два или более нарушений
            if (MIN != 0 && MAX == 0) {
                // Выбор минимального значения из исходных X1, X2, X3
                output_Y = std::min(std::min(X1, X2), X3);
            }
            else if (MIN == 0 && MAX != 0) {
                // Выбор максимального значения из исходных X1, X2, X3
                output_Y = std::max(std::max(X1, X2), X3);
            }
            // В остальных случаях (MIN=0,MAX=0 или MIN=1,MAX=1) оставляем медиану
        }

        // 6. Применение фильтрации для стабилизации выхода
        if (last_Y == 0) {
            last_Y = output_Y;
        }
        else {
            // Простое сглаживание для фильтрации кратковременных возмущений
            Type alpha = 0.1; // Коэффициент сглаживания
            output_Y   = last_Y + alpha * (output_Y - last_Y);
            last_Y     = output_Y;
        }

        // 7. Формирование выходных сигналов
        ports->outputs[0] = output_Y;            // Y
        ports->outputs[1] = ST1_val ? 1.0 : 0.0; // ST1
        ports->outputs[2] = ST2_val ? 1.0 : 0.0; // ST2
        ports->outputs[3] = ST3_val ? 1.0 : 0.0; // ST3
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