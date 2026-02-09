#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <cmath>
#include <sstream>

namespace nrcki {
class PulseDynamic final : public Block {
    using Type = types::real;
    using Time = types::time;

    Ports<Type>* ports;

    mutable bool prev_x = false;
    mutable Time T_off;
    mutable Type prev_T = 0;

public:
    explicit PulseDynamic(const Context& context) :
        Block(context),
        T_off(std::numeric_limits<Time>::min()) {
        // Порты: входной сигнал, параметр T (длительность импульса), выход
        ports = new Ports<Type>(2, 1);
        register_ports(ports);
    }

    void compute() const override {
        const Type current_x = *ports->inputs[0];
        const Type current_T = *ports->inputs[1];

        if (context.time < T_off) {
            ports->outputs[0] = true; // Импульс активен
        }
        else {
            if (!prev_x && current_x != 0) {
                // Фронт входного сигнала - запускаем импульс
                ports->outputs[0] = true;
                T_off             = context.time + static_cast<Time>(current_T * Context::sec);
            }
            else {
                ports->outputs[0] = false; // Импульс неактивен
            }
            prev_x = current_x != 0;
        }

        // Обработка изменения T во время активного импульса
        if (context.time < T_off && prev_T != current_T) {
            Time remaining = T_off - context.time;
            Time old_total = static_cast<Time>(prev_T * Context::sec);
            Time elapsed   = old_total - remaining;
            Time new_total = static_cast<Time>(current_T * Context::sec);

            if (elapsed < new_total) {
                // Пересчитываем время окончания импульса
                T_off = context.time + (new_total - elapsed);
            }
            else {
                // Новое время импульса меньше уже прошедшего - завершаем импульс
                T_off = context.time;
            }
        }

        prev_T = current_T;
    }

    std::string printMemory() const override {
        const auto type = CODE_NAME_TYPE(Time);
        const auto min  = CODE_NAME_MIN(Time);
        REGISTER_VAR(prev_x)
        REGISTER_VAR(prev_T)
        REGISTER_VAR(T_off)

        std::stringstream line;
        line << "bool " << var_prev_x << " = false;\n";
        line << CODE_NAME_TYPE(Type) << ' ' << var_prev_T << " = 0;\n";
        line << type << ' ' << var_T_off << " = " << min << ';';
        return line.str();
    }

    types::string printSource() const override {
        const auto x       = CODE_NAME_IN(ports, 0);
        const auto T_param = CODE_NAME_IN(ports, 1);
        const auto y       = CODE_NAME_OUT(ports, 0);
        const auto min     = CODE_NAME_MIN(Time);
        const auto sec     = CODE_NAME_SEC;

        REGISTER_VAR(prev_x)
        REGISTER_VAR(prev_T)
        REGISTER_VAR(T_off)

        std::stringstream line;
        line << "if (time < " << var_T_off << ") {\n";
        line << "    " << y << " = 1.0;\n";
        line << "}\n";
        line << "else {\n";
        line << "    if (!" << var_prev_x << " && " << x << " != 0) {\n";
        line << "        " << y << " = 1.0;\n";
        line << "        " << var_T_off << " = time + " << T_param << " * " << sec << ";\n";
        line << "    }\n";
        line << "    else {\n";
        line << "        " << y << " = 0.0;\n";
        line << "    }\n";
        line << "    " << var_prev_x << " = " << x << " != 0;\n";
        line << "}\n";
        line << "\n";
        line << "if (time < " << var_T_off << " && " << var_prev_T << " != " << T_param << ") {\n";
        line << "    " << CODE_NAME_TYPE(Time) << " remaining = " << var_T_off << " - time;\n";
        line << "    " << CODE_NAME_TYPE(Time) << " old_total = " << var_prev_T << " * " << sec << ";\n";
        line << "    " << CODE_NAME_TYPE(Time) << " elapsed = old_total - remaining;\n";
        line << "    " << CODE_NAME_TYPE(Time) << " new_total = " << T_param << " * " << sec << ";\n";
        line << "    if (elapsed < new_total) {\n";
        line << "        " << var_T_off << " = time + (new_total - elapsed);\n";
        line << "    }\n";
        line << "    else {\n";
        line << "        " << var_T_off << " = time;\n";
        line << "    }\n";
        line << "}\n";
        line << var_prev_T << " = " << T_param << ";";

        return line.str();
    }
};
}