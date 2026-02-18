#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <algorithm>
#include <sstream>

namespace nrcki {
class NonlinearFilter final : public Block {
    using Type = types::real;

    Ports<Type>* ports;

    // Параметры блока
    Type A;  // Диапазон зоны нечувствительности
    Type K1; // Постоянная линейного передающего звена
    Type K2; // Постоянная нелинейного передающего звена
    Type T;  // Постоянная времени нелинейного фильтра
    Type OG; // Верхний предел выходного сигнала
    Type UG; // Нижний предел выходного сигнала

    // Внутреннее состояние
    mutable Type prev_D;      // Предыдущее значение разностного сигнала
    mutable Type prev_Y;      // Предыдущее значение выходного сигнала
    mutable Type integral;    // Значение интегратора
    mutable bool initialized; // Флаг инициализации

public:
    explicit NonlinearFilter(const Context& context,
                             Type A = 0, Type K1 = 1, Type K2 = 0.5,
                             Type T = 1, Type OG = 0, Type UG = 0) :
        Block(context),
        A(std::max(A, static_cast<Type>(0.0001))),                              // A должно быть > 0
        K1(std::clamp(K1, static_cast<Type>(0.001), static_cast<Type>(0.999))), // 0 < K1 < 1
        K2(std::clamp(K2, static_cast<Type>(1.001), static_cast<Type>(9.999))), // 1 < K2 < 10
        T(std::max(T, static_cast<Type>(0.101))),                               // T > 0.1
        OG(OG), UG(UG),
        prev_D(0), prev_Y(0), integral(0), initialized(false) {
        ports = new Ports<Type>(1, 1); // 1 вход, 1 выход
        register_ports(ports);
    }

    void compute() const override {
        // Получаем входной сигнал
        Type X = *ports->inputs[0];

        // Инициализация при первом вызове
        if (!initialized) {
            prev_Y      = X; // Начальное значение выхода равно входу
            prev_D      = 0; // Начальная разность 0
            integral    = X; // Интегратор инициализируется значением X
            initialized = true;
        }

        // 1. Вычисление разностного сигнала D = X - Y
        Type D = X - prev_Y;

        // 2. Линейное передающее звено LG
        // Сжатие разности между текущим и предыдущим значением D
        Type LG_out = K1 * (D - prev_D);

        // 3. Нелинейное передающее звено NLG
        Type NLG_out = 0;
        Type abs_D   = std::abs(D);

        if (abs_D <= A) {
            // В зоне нечувствительности: большой коэффициент K2
            NLG_out = K2 * D;
        }
        else {
            // За пределами зоны нечувствительности
            if (D > 0) {
                // Часть в зоне нечувствительности с коэффициентом K2
                // и часть за зоной с коэффициентом 1
                NLG_out = K2 * A + (D - A);
            }
            else {
                NLG_out = K2 * (-A) + (D + A);
            }
        }

        // 4. Суммирование выходов LG и NLG
        Type sum_LG_NLG = LG_out + NLG_out;

        // 5. Определение переменной постоянной времени интегратора
        // Постоянная времени зависит от разностного сигнала D и параметра A
        Type T_int = 0;
        if (A > 0) {
            // T_int = T * (1 + |D| / A)
            T_int = T * (1.0 + abs_D / A);
        }
        else {
            T_int = T;
        }

        // 6. Интегрирование с переменной постоянной времени
        // Интегратор: Y = ∫(sum_LG_NLG / T_int) dt
        if (T_int > 0) {
            integral += (sum_LG_NLG / T_int) * Context::dt_sec;
        }

        // 7. Формирование выходного сигнала
        Type Y = integral;

        // 8. Применение ограничений выходного сигнала
        if (OG != 0 || UG != 0) {
            // Если заданы ограничения
            if (OG > UG) {
                // Проверка корректности ограничений
                Y = std::clamp(Y, UG, OG);
            }
        }

        // 9. Обновление внутреннего состояния
        prev_D = D;
        prev_Y = Y;

        // 10. Установка выходного сигнала
        ports->outputs[0] = Y;
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