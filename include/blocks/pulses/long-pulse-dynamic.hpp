#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <cmath>
#include <sstream>

namespace nrcki {
class LongPulseDynamic final : public Block {
    using Type = types::real;
    using Time = types::time;

    Ports<Type>* ports;

    mutable bool prev_x = false;
    mutable Time T_off;
    mutable Time time_start;
    mutable Type prev_T = 0;

public:
    explicit LongPulseDynamic(const Context& context) :
        Block(context),
        T_off(std::numeric_limits<Time>::min()),
        time_start(0) {
        ports = new Ports<Type>(2, 1);
        register_ports(ports);
    }

    void compute() const override {
        // Обнаружение фронта входного сигнала
        if (!prev_x && *ports->inputs[0] != 0) {
            time_start = context.time;
            T_off      = time_start + static_cast<Time>(*ports->inputs[1] * Context::sec);
        }

        // Обновление времени окончания импульса при изменении T
        if (prev_T != *ports->inputs[1] && context.time < T_off) {
            Time elapsed   = context.time - time_start;
            Time new_total = static_cast<Time>(*ports->inputs[1] * Context::sec);

            if (elapsed < new_total) {
                T_off = time_start + new_total;
            }
            else {
                T_off = context.time;
            }
        }

        prev_x            = *ports->inputs[0] != 0;
        prev_T            = *ports->inputs[1];
        ports->outputs[0] = context.time < T_off;
    }

    std::string printMemory() const override {
        const auto type = CODE_NAME_TYPE(Time);
        const auto min  = CODE_NAME_MIN(Time);
        REGISTER_VAR(prev_x)
        REGISTER_VAR(time_start)
        REGISTER_VAR(prev_T)
        REGISTER_VAR(T_off)

        std::stringstream line;
        line << "bool " << var_prev_x << " = false;\n";
        line << type << ' ' << var_time_start << " = 0;\n";
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
        REGISTER_VAR(time_start)
        REGISTER_VAR(prev_T)
        REGISTER_VAR(T_off)

        std::stringstream line;
        line << "if (!" << var_prev_x << " && " << x << " != 0) {\n";
        line << "    " << var_time_start << " = time;\n";
        line << "    " << var_T_off << " = " << var_time_start << " + " << T_param << " * " << sec << ";\n";
        line << "}\n";
        line << "\n";
        line << "if (" << var_prev_T << " != " << T_param << " && time < " << var_T_off << ") {\n";
        line << "    " << CODE_NAME_TYPE(Time) << " elapsed = time - " << var_time_start << ";\n";
        line << "    " << CODE_NAME_TYPE(Time) << " new_total = " << T_param << " * " << sec << ";\n";
        line << "    if (elapsed < new_total) {\n";
        line << "        " << var_T_off << " = " << var_time_start << " + new_total;\n";
        line << "    }\n";
        line << "    else {\n";
        line << "        " << var_T_off << " = time;\n";
        line << "    }\n";
        line << "}\n";
        line << "\n";
        line << var_prev_x << " = " << x << " != 0;\n";
        line << var_prev_T << " = " << T_param << ";\n";
        line << y << " = time < " << var_T_off << ';';

        return line.str();
    }
};
}