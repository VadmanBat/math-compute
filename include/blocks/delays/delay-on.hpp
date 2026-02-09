#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <sstream>
#include <cmath>

namespace nrcki {
class DelayOn final : public Block {
    using Type = types::real;
    using Time = types::time;

    Ports<Type>* ports;

    const Time T;
    mutable Time T_on;
    mutable bool prev_x = false;

public:
    explicit DelayOn(const Context& context, const Type& T) :
        Block(context),
        T(static_cast<Time>(T * Context::sec)),
        T_on(std::numeric_limits<Time>::max()) {
        ports = new Ports<Type>(1, 1);
        register_ports(ports);
    }

    void compute() const override {
        if (prev_x != (*ports->inputs[0] != 0))
            T_on = !prev_x ? context.time + T : std::numeric_limits<Time>::max();
        prev_x            = *ports->inputs[0] != 0;
        ports->outputs[0] = context.time >= T_on;
    }

    std::string printMemory() const override {
        const auto type = CODE_NAME_TYPE(Time);
        const auto max  = CODE_NAME_MAX(Time);
        REGISTER_VAR(prev_x)
        REGISTER_VAR(T_on)

        std::stringstream line;
        line << "bool " << var_prev_x << " = false;\n";
        line << type << ' ' << var_T_on << " = " << max << ';';
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
        line << "if (" << var_prev_x << " != (" << x << " != 0))\n";
        line << "    " << var_T_on << " = !" << var_prev_x << " ? time + " << T << " : " << max << ";\n";
        line << var_prev_x << " = " << x << " != 0;\n";
        line << y << " = time >= " << var_T_on << ';';
        return line.str();
    }
};
}