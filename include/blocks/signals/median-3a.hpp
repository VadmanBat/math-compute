#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <algorithm>
#include <sstream>

namespace nrcki {
class Median3A final : public Block {
    using Type = types::real;

    Ports<Type>* ports;

    // Параметры блока
    Type TOL;  // Предельно допустимое рассогласование
    Type BMIN; // Алгоритм при недостоверных сигналах - выбор минимального
    Type BMAX; // Алгоритм при недостоверных сигналах - выбор максимального

    // Структура для хранения значения с его индексом
    struct ValueWithIndex {
        Type value;
        int index; // 0: X1, 1: X2, 2: X3
        bool operator<(const ValueWithIndex& other) const {
            return value < other.value;
        }
    };

public:
    explicit Median3A(const Context& context,
                      Type TOL = 2, Type BMIN = 1, Type BMAX = 0) :
        Block(context),
        TOL(TOL), BMIN(BMIN), BMAX(BMAX) {
        ports = new Ports<Type>(6, 7); // 6 входов, 7 выходов
        register_ports(ports);
    }

    void compute() const override {
        // Получаем входные сигналы
        Type X1  = *ports->inputs[0];
        Type X2  = *ports->inputs[1];
        Type X3  = *ports->inputs[2];
        Type FG1 = *ports->inputs[3];
        Type FG2 = *ports->inputs[4];
        Type FG3 = *ports->inputs[5];

        // 1. Определение максимального, медианного и минимального значений
        ValueWithIndex values[3] = {
            {X1, 0},
            {X2, 1},
            {X3, 2}
        };

        // Создаем копию для сортировки
        ValueWithIndex sorted[3] = {values[0], values[1], values[2]};
        std::sort(sorted, sorted + 3);

        Type min_val    = sorted[0].value;
        Type median_val = sorted[1].value;
        Type max_val    = sorted[2].value;

        // 2. Определение признаков отказа ABWX1, ABWX2, ABWX3
        bool ABWX1_val = false;
        bool ABWX2_val = false;
        bool ABWX3_val = false;

        // Проверяем разности
        Type diff_max_med = max_val - median_val;
        Type diff_med_min = median_val - min_val;

        if (diff_max_med > TOL) {
            // Максимальное значение отклоняется
            if (sorted[2].index == 0)
                ABWX1_val = true;
            else if (sorted[2].index == 1)
                ABWX2_val = true;
            else
                ABWX3_val = true;
        }

        if (diff_med_min > TOL) {
            // Минимальное значение отклоняется
            if (sorted[0].index == 0)
                ABWX1_val = true;
            else if (sorted[0].index == 1)
                ABWX2_val = true;
            else
                ABWX3_val = true;
        }

        // 3. Определение достоверности входных сигналов
        // Достоверность = (FG != 0) И (ABWX? == false)
        bool valid[3] = {
            (FG1 != 0) && !ABWX1_val,
            (FG2 != 0) && !ABWX2_val,
            (FG3 != 0) && !ABWX3_val
        };

        // Подсчет достоверных сигналов
        int valid_count = 0;
        for (int i = 0; i < 3; i++) {
            if (valid[i])
                valid_count++;
        }

        // 4. Формирование выходного значения Y в зависимости от количества достоверных сигналов
        Type output_Y = 0;

        if (valid_count == 3) {
            // Случай (а): все три достоверны - медианное значение
            output_Y = median_val;
        }
        else if (valid_count == 2) {
            // Случай (б): два достоверны, один недостоверен
            // Определяем, какие именно достоверны
            Type valid_values[2];
            int idx = 0;

            for (int i = 0; i < 3; i++)
                if (valid[i])
                    valid_values[idx++] = values[i].value;

            if (BMIN != 0 && BMAX == 0) {
                // Выбор минимального из двух достоверных
                output_Y = std::min(valid_values[0], valid_values[1]);
            }
            else if (BMIN == 0 && BMAX != 0) {
                // Выбор максимального из двух достоверных
                output_Y = std::max(valid_values[0], valid_values[1]);
            }
            else {
                // BMIN=0,BMAX=0 или BMIN=1,BMAX=1 - медианное из всех трех
                output_Y = median_val;
            }
        }
        else {
            // Случай (в): два или три недостоверны (0 или 1 достоверный)
            if (BMIN != 0 && BMAX == 0) {
                // Выбор минимального из всех трех
                output_Y = min_val;
            }
            else if (BMIN == 0 && BMAX != 0) {
                // Выбор максимального из всех трех
                output_Y = max_val;
            }
            else {
                // BMIN=0,BMAX=0 или BMIN=1,BMAX=1 - медианное из всех трех
                output_Y = median_val;
            }
        }

        // 5. Формирование выходных дискретных сигналов
        ports->outputs[0] = output_Y;                       // Y
        ports->outputs[1] = (valid_count >= 2) ? 1.0 : 0.0; // FG
        ports->outputs[2] = (valid_count < 2) ? 1.0 : 0.0;  // S2V3
        ports->outputs[3] = (valid_count < 3) ? 1.0 : 0.0;  // S1V3
        ports->outputs[4] = ABWX1_val ? 1.0 : 0.0;          // ABWX1
        ports->outputs[5] = ABWX2_val ? 1.0 : 0.0;          // ABWX2
        ports->outputs[6] = ABWX3_val ? 1.0 : 0.0;          // ABWX3
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