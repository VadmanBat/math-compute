#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <cmath>
#include <sstream>

namespace nrcki {
class LongPulse final : public Block {
    using Type = types::real;
    using Time = types::time;

    Ports<Type>* ports;

    const Time T;
    mutable bool prev_x = false;
    mutable Time T_off;

public:
    explicit LongPulse(const Context& context, const Type& T) :
        Block(context),
        T(static_cast<Time>(T * Context::sec)),
        T_off(std::numeric_limits<Time>::min()) {
        ports = new Ports<Type>(1, 1);
        register_ports(ports);
    }

    void compute() const override {
        if (!prev_x && *ports->inputs[0] != 0)
            T_off = context.time + T;
        prev_x            = *ports->inputs[0] != 0;
        ports->outputs[0] = context.time < T_off;
    }

    types::string printMemory() const override {
        const auto type = CODE_NAME_TYPE(Time);
        const auto min  = CODE_NAME_MIN(Time);
        REGISTER_VAR(prev_x)
        REGISTER_VAR(T_off)

        std::stringstream line;
        line << "bool " << var_prev_x << " = false;\n";
        line << type << ' ' << var_T_off << " = " << min << ';';
        return line.str();
    }

    types::string printSource() const override {
        const auto x = CODE_NAME_IN(ports, 0);
        const auto y = CODE_NAME_OUT(ports, 0);
        REGISTER_VAR(prev_x)
        REGISTER_VAR(T_off)

        std::stringstream line;
        line << "if (!" << var_prev_x << " && " << x << "!= 0)\n";
        line << "    " << var_T_off << " = time + " << T << ";\n";
        line << var_prev_x << " = " << x << " != 0;\n";
        line << y << " = time < " << var_T_off << ';';
        return line.str();
    }
};
}