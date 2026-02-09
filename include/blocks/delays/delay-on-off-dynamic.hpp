#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <sstream>
#include <cmath>

namespace nrcki {
class DelayOnOffDynamic final : public Block {
    using Type = types::real;
    using Time = types::time;

    Ports<Type>* ports;

    mutable Time T_on, T_off;
    mutable bool prev_x           = false;
    mutable Type prev_T_on        = 0;
    mutable Type prev_T_off       = 0;
    mutable bool timer_on_active  = false;
    mutable bool timer_off_active = false;

public:
    explicit DelayOnOffDynamic(const Context& context) :
        Block(context),
        T_on(std::numeric_limits<Time>::max()),
        T_off(std::numeric_limits<Time>::min()) {
        // Порты: входной сигнал, T_on, T_off, выход
        ports = new Ports<Type>(3, 1);
        register_ports(ports);
    }

    void compute() const override {
        Type current_x     = *ports->inputs[0];
        Type current_T_on  = *ports->inputs[1];
        Type current_T_off = *ports->inputs[2];

        bool x_changed     = prev_x != (current_x != 0);
        bool T_on_changed  = prev_T_on != current_T_on;
        bool T_off_changed = prev_T_off != current_T_off;

        if (x_changed) {
            if (ports->outputs[0] != 0) {
                if (prev_x) {
                    if (T_off < context.time || T_on <= context.time) {
                        T_off            = context.time + static_cast<Time>(current_T_off * Context::sec);
                        timer_off_active = true;
                    }
                    T_on            = std::numeric_limits<Time>::max();
                    timer_on_active = false;
                }
                else {
                    T_on            = context.time + static_cast<Time>(current_T_on * Context::sec);
                    timer_on_active = true;
                }
            }
            else {
                T_on = prev_x
                           ? std::numeric_limits<Time>::max()
                           : context.time + static_cast<Time>(current_T_on * Context::sec);
                timer_on_active = !prev_x;
            }
        }
        else {
            // Обработка изменения T_on при активном таймере включения
            if (T_on_changed && timer_on_active && T_on != std::numeric_limits<Time>::max()) {
                Time remaining = T_on - context.time;
                Time new_total = static_cast<Time>(current_T_on * Context::sec);
                if (remaining > 0) {
                    // Пересчитываем T_on на основе нового времени
                    Time elapsed = static_cast<Time>(prev_T_on * Context::sec) - remaining;
                    if (elapsed < new_total) {
                        T_on = context.time + (new_total - elapsed);
                    }
                    else {
                        T_on = context.time; // Таймер уже должен был сработать
                    }
                }
            }

            // Обработка изменения T_off при активном таймере выключения
            if (T_off_changed && timer_off_active && T_off != std::numeric_limits<Time>::min()) {
                Time remaining = T_off - context.time;
                Time new_total = static_cast<Time>(current_T_off * Context::sec);
                if (remaining > 0) {
                    // Пересчитываем T_off на основе нового времени
                    Time elapsed = static_cast<Time>(prev_T_off * Context::sec) - remaining;
                    if (elapsed < new_total) {
                        T_off = context.time + (new_total - elapsed);
                    }
                    else {
                        T_off = context.time; // Таймер уже должен был сработать
                    }
                }
            }
        }

        prev_x     = current_x != 0;
        prev_T_on  = current_T_on;
        prev_T_off = current_T_off;

        // Обновляем состояние таймеров
        timer_on_active  = T_on != std::numeric_limits<Time>::max() && context.time < T_on;
        timer_off_active = T_off != std::numeric_limits<Time>::min() && context.time < T_off;

        // Выход = 1, если текущее время >= T_on или текущее время < T_off
        ports->outputs[0] = context.time >= T_on || context.time < T_off;
    }

    std::string printMemory() const override {
        const auto type = CODE_NAME_TYPE(Time);
        const auto min  = CODE_NAME_MIN(Time);
        const auto max  = CODE_NAME_MAX(Time);
        REGISTER_VAR(prev_x)
        REGISTER_VAR(prev_T_on)
        REGISTER_VAR(prev_T_off)
        REGISTER_VAR(timer_on_active)
        REGISTER_VAR(timer_off_active)
        REGISTER_VAR(T_on)
        REGISTER_VAR(T_off)

        std::stringstream line;
        line << "bool " << var_prev_x << " = false;\n";
        line << CODE_NAME_TYPE(Type) << ' ' << var_prev_T_on << " = 0;\n";
        line << CODE_NAME_TYPE(Type) << ' ' << var_prev_T_off << " = 0;\n";
        line << "bool " << var_timer_on_active << " = false;\n";
        line << "bool " << var_timer_off_active << " = false;\n";
        line << type << ' ' << var_T_on << " = " << max << ", " << var_T_off << " = " << min << ';';
        return line.str();
    }

    types::string printSource() const override {
        const auto type        = CODE_NAME_TYPE(Time);
        const auto x           = CODE_NAME_IN(ports, 0);
        const auto T_on_param  = CODE_NAME_IN(ports, 1);
        const auto T_off_param = CODE_NAME_IN(ports, 2);
        const auto y           = CODE_NAME_OUT(ports, 0);
        const auto min         = CODE_NAME_MIN(Time);
        const auto max         = CODE_NAME_MAX(Time);
        const auto sec         = CODE_NAME_SEC;

        REGISTER_VAR(prev_x)
        REGISTER_VAR(prev_T_on)
        REGISTER_VAR(prev_T_off)
        REGISTER_VAR(timer_on_active)
        REGISTER_VAR(timer_off_active)
        REGISTER_VAR(T_on)
        REGISTER_VAR(T_off)

        std::stringstream line;
        line << "{\n";
        line << "bool x_changed = " << var_prev_x << " != (" << x << " != 0);\n";
        line << "bool T_on_changed = " << var_prev_T_on << " != " << T_on_param << ";\n";
        line << "bool T_off_changed = " << var_prev_T_off << " != " << T_off_param << ";\n";
        line << "\n";
        line << "if (x_changed) {\n";
        line << "    if (" << y << " != 0) {\n";
        line << "        if (" << var_prev_x << ") {\n";
        line << "            if (" << var_T_off << " < time || " << var_T_on << " <= time) {\n";
        line << "                " << var_T_off << " = time + " << T_off_param << " * " << sec << ";\n";
        line << "                " << var_timer_off_active << " = true;\n";
        line << "            }\n";
        line << "            " << var_T_on << " = " << max << ";\n";
        line << "            " << var_timer_on_active << " = false;\n";
        line << "        }\n";
        line << "        else {\n";
        line << "            " << var_T_on << " = time + " << T_on_param << " * " << sec << ";\n";
        line << "            " << var_timer_on_active << " = true;\n";
        line << "        }\n";
        line << "    }\n";
        line << "    else {\n";
        line << "        " << var_T_on << " = " << var_prev_x << " ? " << max << " : time + " << T_on_param << " * " <<
            sec << ";\n";
        line << "        " << var_timer_on_active << " = !" << var_prev_x << ";\n";
        line << "    }\n";
        line << "}\n";
        line << "else {\n";
        line << "    if (T_on_changed && " << var_timer_on_active << " && " << var_T_on << " != " << max << ") {\n";
        line << "        " << type << " remaining = " << var_T_on << " - time;\n";
        line << "        " << type << " new_total = " << T_on_param << " * " << sec << ";\n";
        line << "        if (remaining > 0) {\n";
        line << "            " << type << " elapsed = " << var_prev_T_on << " * " << sec << " - remaining;\n";
        line << "            if (elapsed < new_total) {\n";
        line << "                " << var_T_on << " = time + (new_total - elapsed);\n";
        line << "            }\n";
        line << "            else {\n";
        line << "                " << var_T_on << " = time;\n";
        line << "            }\n";
        line << "        }\n";
        line << "    }\n";
        line << "\n";
        line << "    if (T_off_changed && " << var_timer_off_active << " && " << var_T_off << " != " << min << ") {\n";
        line << "        " << type << " remaining = " << var_T_off << " - time;\n";
        line << "        " << type << " new_total = " << T_off_param << " * " << sec << ";\n";
        line << "        if (remaining > 0) {\n";
        line << "            " << type << " elapsed = " << var_prev_T_off << " * " << sec << " - remaining;\n";
        line << "            if (elapsed < new_total) {\n";
        line << "                " << var_T_off << " = time + (new_total - elapsed);\n";
        line << "            }\n";
        line << "            else {\n";
        line << "                " << var_T_off << " = time;\n";
        line << "            }\n";
        line << "        }\n";
        line << "    }\n";
        line << "}\n";
        line << "\n";
        line << var_prev_x << " = " << x << " != 0;\n";
        line << var_prev_T_on << " = " << T_on_param << ";\n";
        line << var_prev_T_off << " = " << T_off_param << ";\n";
        line << var_timer_on_active << " = " << var_T_on << " != " << max << " && time < " << var_T_on << ";\n";
        line << var_timer_off_active << " = " << var_T_off << " != " << min << " && time < " << var_T_off << ";\n";
        line << y << " = time >= " << var_T_on << " || time < " << var_T_off << ";\n";
        line << '}';

        return line.str();
    }
};
}