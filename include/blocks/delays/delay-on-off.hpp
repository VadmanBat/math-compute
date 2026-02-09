#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <sstream>
#include <cmath>

namespace nrcki {
class DelayOnOff final : public Block {
    using Type = types::real;
    using Time = types::time;

    Ports<Type>* ports;

    const Time P_on, P_off;
    mutable Time T_on, T_off;
    mutable bool prev_x = false;

public:
    explicit DelayOnOff(const Context& context, const Type& T_on, const Type& T_off) :
        Block(context),
        P_on(static_cast<Time>(T_on * Context::sec)),
        P_off(static_cast<Time>(T_off * Context::sec)),
        T_on(std::numeric_limits<Time>::max()),
        T_off(std::numeric_limits<Time>::min()) {
        ports = new Ports<Type>(1, 1);
        register_ports(ports);
    }

    void compute() const override {
        if (prev_x != (*ports->inputs[0] != 0)) {
            if (ports->outputs[0] != 0) {
                if (prev_x) {
                    if (T_off < context.time || T_on <= context.time)
                        T_off = context.time + P_off;
                    T_on = std::numeric_limits<Time>::max();
                }
                else {
                    T_on = context.time + P_on;
                }
            }
            else {
                T_on = prev_x ? std::numeric_limits<Time>::max() : context.time + P_on;
            }
        }

        prev_x            = *ports->inputs[0] != 0;
        ports->outputs[0] = context.time >= T_on || context.time < T_off;
    }

    std::string printMemory() const override {
        const auto type = CODE_NAME_TYPE(Time);
        const auto min  = CODE_NAME_MIN(Time);
        const auto max  = CODE_NAME_MAX(Time);
        REGISTER_VAR(prev_x)
        REGISTER_VAR(T_on)
        REGISTER_VAR(T_off)

        std::stringstream line;
        line << "bool " << var_prev_x << " = false;\n";
        line << type << ' ' << var_T_on << " = " << max << ", " << var_T_off << " = " << min << ';';
        return line.str();
    }

    types::string printSource() const override {
        const auto x   = CODE_NAME_IN(ports, 0);
        const auto y   = CODE_NAME_OUT(ports, 0);
        const auto max = CODE_NAME_MAX(Time);
        REGISTER_VAR(prev_x)
        REGISTER_VAR(T_on)
        REGISTER_VAR(T_off)

        std::stringstream line;
        line << "if (" << var_prev_x << " != (" << x << " != 0)) {\n";
        line << "    if (" << y << " != 0) {\n";
        line << "        if (" << var_prev_x << ") {\n";
        line << "            if (" << var_T_off << " < time || " << var_T_on << " <= time)\n";
        line << "                " << var_T_off << " = time + " << P_off << ";\n";
        line << "            " << var_T_on << " = " << max << ";\n";
        line << "        }\n";
        line << "        else {\n";
        line << "            " << var_T_on << " = time + " << P_on << ";\n";
        line << "        }\n";
        line << "    }\n";
        line << "    else {\n";
        line << "        " << var_T_on << " = " << var_prev_x << " ? " << max << " : time + " << P_on << ";\n";
        line << "    }\n";
        line << "}\n";
        line << var_prev_x << " = " << x << " != 0;\n";
        line << y << " = time >= " << var_T_on << " || time < " << var_T_off << ';';
        return line.str();
    }
};
}