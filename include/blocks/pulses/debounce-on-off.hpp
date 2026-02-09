#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <cmath>
#include <sstream>

namespace nrcki {
class DebounceOnOff final : public Block {
    using Type = types::real;
    using Time = types::time;

    Ports<Type>* ports;

    const Time T;
    mutable bool prev_x = false;
    mutable Time T_on, T_off;

public:
    explicit DebounceOnOff(const Context& context, const Type& T) :
        Block(context),
        T(static_cast<Time>(T * Context::sec)),
        T_on(std::numeric_limits<Time>::max()),
        T_off(std::numeric_limits<Time>::min()) {
        ports = new Ports<Type>(1, 1);
        register_ports(ports);
    }

    void compute() const override {
        if (prev_x != (*ports->inputs[0] != 0)) {
            ports->outputs[0] = context.time >= T_on && context.time < T_off;
            if (!prev_x) {
                T_on  = ports->outputs[0] == 0 ? context.time + T : std::numeric_limits<Time>::min();
                T_off = std::numeric_limits<Time>::max();
            }
            else {
                if (ports->outputs[0] != 0) {
                    T_on  = std::numeric_limits<Time>::min();
                    T_off = context.time + T;
                }
                else {
                    T_on  = std::numeric_limits<Time>::max();
                    T_off = std::numeric_limits<Time>::min();
                }
            }
            prev_x = *ports->inputs[0] != 0;
        }
        else
            ports->outputs[0] = context.time >= T_on && context.time < T_off;
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
        const auto min = CODE_NAME_MIN(Time);
        const auto max = CODE_NAME_MAX(Time);
        REGISTER_VAR(prev_x)
        REGISTER_VAR(T_on)
        REGISTER_VAR(T_off)

        std::stringstream line;
        line << "if (" << var_prev_x << " != (" << x << "!= 0)) {\n";
        line << "    if (!" << var_prev_x << ") {\n";
        line << "        " << var_T_on << " = " << y << " == 0 ? time + " << T << " : " << min << ";\n";
        line << "        " << var_T_off << " = " << max << ";\n";
        line << "    }\n";
        line << "    else {\n";
        line << "        if (" << y << " != 0) {\n";
        line << "            " << var_T_on << " = " << min << ";\n";
        line << "            " << var_T_off << " = time + " << T << ";\n";
        line << "        }\n";
        line << "        else {\n";
        line << "            " << var_T_on << " = " << max << ";\n";
        line << "            " << var_T_off << " = " << min << ";\n";
        line << "        }\n";
        line << "    }\n";
        line << "    " << var_prev_x << " = " << x << " != 0;\n";
        line << "}\n";
        line << y << " = time >= " << var_T_on << " && time < " << var_T_on << ';';
        return line.str();
    }
};
}