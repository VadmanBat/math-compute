#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <sstream>
#include <cmath>

namespace nrcki {
class DelayOnDynamic final : public Block {
    using Type = types::real;
    using Time = types::time;

    Ports<Type>* ports;

    mutable Time T_on;
    mutable Time time_start;   // Время начала отсчета задержки
    mutable Type prev_T;       // Предыдущее значение T для отслеживания изменений
    mutable bool timer_active; // Флаг активности таймера

public:
    explicit DelayOnDynamic(const Context& context) :
        Block(context),
        T_on(std::numeric_limits<Time>::max()),
        time_start(0),
        prev_T(0),
        timer_active(false) {
        // Порты: входной сигнал, параметр T, выход
        ports = new Ports<Type>(2, 1);
        register_ports(ports);
    }

    void compute() const override {
        const Type current_x = *ports->inputs[0];
        const Type current_T = *ports->inputs[1];

        bool x_changed = (current_x != 0) != timer_active;

        if (x_changed) {
            if (current_x != 0) {
                // Входной сигнал стал ненулевым - запускаем таймер
                timer_active = true;
                time_start   = context.time;
                T_on         = time_start + static_cast<Time>(current_T * Context::sec);
            }
            else {
                // Входной сигнал стал нулевым - сбрасываем таймер
                timer_active = false;
                T_on         = std::numeric_limits<Time>::max();
            }
        }
        else if (timer_active && prev_T != current_T) {
            // T изменился во время работы таймера - пересчитываем оставшееся время
            Time elapsed = context.time - time_start; // Прошедшее время
            Type old_T   = prev_T;

            if (current_T > old_T) {
                // Увеличили T - продлеваем таймер
                Time remaining_old   = static_cast<Time>(old_T * Context::sec) - elapsed;
                Time additional_time = static_cast<Time>((current_T - old_T) * Context::sec);
                T_on                 = context.time + remaining_old + additional_time;
            }
            else {
                // Уменьшили T - возможно, таймер уже истек
                Time new_total = static_cast<Time>(current_T * Context::sec);
                if (elapsed < new_total) {
                    // Таймер еще не истек
                    T_on = time_start + new_total;
                }
                else {
                    // Таймер уже истек
                    T_on = context.time; // Устанавливаем текущее время
                }
            }
        }

        prev_T = current_T;

        // Выход = true, если таймер активен и текущее время больше или равно времени включения
        ports->outputs[0] = timer_active && context.time >= T_on;
    }

    std::string printMemory() const override {
        const auto type = CODE_NAME_TYPE(Time);
        const auto max  = CODE_NAME_MAX(Time);
        REGISTER_VAR(timer_active)
        REGISTER_VAR(time_start)
        REGISTER_VAR(prev_T)
        REGISTER_VAR(T_on)

        std::stringstream line;
        line << "bool " << var_timer_active << " = false;\n";
        line << type << ' ' << var_time_start << " = 0;\n";
        line << CODE_NAME_TYPE(Type) << ' ' << var_prev_T << " = 0;\n";
        line << type << ' ' << var_T_on << " = " << max << ';';
        return line.str();
    }

    types::string printSource() const override {
        const auto type    = CODE_NAME_TYPE(Time);
        const auto x       = CODE_NAME_IN(ports, 0);
        const auto T_param = CODE_NAME_IN(ports, 1);
        const auto y       = CODE_NAME_OUT(ports, 0);
        const auto max     = CODE_NAME_MAX(Time);
        const auto sec     = CODE_NAME_SEC;

        REGISTER_VAR(timer_active)
        REGISTER_VAR(time_start)
        REGISTER_VAR(prev_T)
        REGISTER_VAR(T_on)

        std::stringstream line;
        line << "{\n";
        line << "bool x_changed = (" << x << " != 0) != " << var_timer_active << ";\n";
        line << "\n";
        line << "if (x_changed) {\n";
        line << "    if (" << x << " != 0) {\n";
        line << "        " << var_timer_active << " = true;\n";
        line << "        " << var_time_start << " = time;\n";
        line << "        " << var_T_on << " = " << var_time_start << " + " << T_param << " * " << sec << ";\n";
        line << "    }\n";
        line << "    else {\n";
        line << "        " << var_timer_active << " = false;\n";
        line << "        " << var_T_on << " = " << max << ";\n";
        line << "    }\n";
        line << "}\n";
        line << "else if (" << var_timer_active << " && " << var_prev_T << " != " << T_param << ") {\n";
        line << "    " << type << " elapsed = time - " << var_time_start << ";\n";
        line << "    " << CODE_NAME_TYPE(Type) << " old_T = " << var_prev_T << ";\n";
        line << "    if (" << T_param << " > old_T) {\n";
        line << "        " << type << " remaining_old = old_T * " << sec << " - elapsed;\n";
        line << "        " << type << " additional_time = (" << T_param << " - old_T) * " << sec << ";\n";
        line << "        " << var_T_on << " = time + remaining_old + additional_time;\n";
        line << "    }\n";
        line << "    else {\n";
        line << "        " << type << " new_total = " << T_param << " * " << sec << ";\n";
        line << "        if (elapsed < new_total) {\n";
        line << "            " << var_T_on << " = " << var_time_start << " + new_total;\n";
        line << "        }\n";
        line << "        else {\n";
        line << "            " << var_T_on << " = time;\n";
        line << "        }\n";
        line << "    }\n";
        line << "}\n";
        line << "\n";
        line << var_prev_T << " = " << T_param << ";\n";
        line << y << " = " << var_timer_active << " && time >= " << var_T_on << ";\n";
        line << '}';

        return line.str();
    }
};
}