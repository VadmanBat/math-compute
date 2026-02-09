#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <sstream>

namespace nrcki {
class Mean final : public Block {
    using Type = types::real;

    Ports<Type>* ports;

    // Параметры достоверности входных значений
    Type KFG1; // Признак достоверности X1
    Type KFG2; // Признак достоверности X2
    Type KFG3; // Признак достоверности X3

    // Параметры для каскадирования (для обработки более 3 значений)
    Type UE;   // Сумма аналоговых значений от предыдущего блока
    Type QU;   // Количество аналоговых значений от предыдущего блока
    Type FGUE; // Признак достоверности суммы от предыдущего блока

public:
    explicit Mean(const Context& context,
                  Type KFG1 = 1, Type KFG2 = 1, Type KFG3 = 1,
                  Type UE   = 0, Type QU   = 0, Type FGUE = 0) :
        Block(context),
        KFG1(KFG1), KFG2(KFG2), KFG3(KFG3),
        UE(UE), QU(QU), FGUE(FGUE) {
        ports = new Ports<Type>(3, 4); // 3 аналоговых входа, 4 выхода
        register_ports(ports);
    }

    void compute() const override {
        // Получаем входные сигналы
        Type X1 = *ports->inputs[0];
        Type X2 = *ports->inputs[1];
        Type X3 = *ports->inputs[2];

        // 1. Вычисление суммы и количества достоверных входных значений
        Type local_sum   = 0;
        Type local_count = 0;

        // Проверяем достоверность каждого входа по соответствующему параметру
        if (KFG1 != 0) {
            local_sum   += X1;
            local_count += 1;
        }

        if (KFG2 != 0) {
            local_sum   += X2;
            local_count += 1;
        }

        if (KFG3 != 0) {
            local_sum   += X3;
            local_count += 1;
        }

        // 2. Учет значений от предыдущего блока (для каскадирования)
        Type total_sum   = local_sum;
        Type total_count = local_count;

        // Если сумма от предыдущего блока достоверна, добавляем её
        if (FGUE != 0) {
            total_sum   += UE;
            total_count += QU;
        }

        // 3. Вычисление среднего арифметического
        Type average = 0;
        if (total_count > 0) {
            average = total_sum / total_count;
        }

        // 4. Формирование выходных сигналов
        ports->outputs[0] = total_sum;   // SUM - сумма всех достоверных значений
        ports->outputs[1] = total_count; // N - количество достоверных значений

        // Y - среднее арифметическое
        ports->outputs[2] = average;

        // FG - признак достоверности среднего
        // Согласно описанию: FG является признаком достоверности выходного сигнала
        // Выходной сигнал считается достоверным, если хотя бы одно значение достоверно
        // (или для каскадирования - если предыдущая сумма достоверна или текущие значения достоверны)
        if (FGUE != 0) {
            // Если есть достоверные значения от предыдущего блока
            ports->outputs[3] = 1.0;
        }
        else if (local_count > 0) {
            // Если есть достоверные текущие значения
            ports->outputs[3] = 1.0;
        }
        else {
            // Нет достоверных значений
            ports->outputs[3] = 0.0;
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