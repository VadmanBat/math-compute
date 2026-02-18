#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <sstream>

namespace nrcki {
class TTriggerB final : public Block {
    using Type = types::real;

    Ports<Type>* ports;

    const bool y0;
    mutable bool prev_x = false;

public:
    explicit TTriggerB(const Context& context, const bool y0 = false) :
        Block(context),
        y0(y0) {
        ports = new Ports<Type>(1, 1);
        register_ports(ports);
    }

    void init() const override {
        prev_x            = *ports->inputs[0] != 0;
        ports->outputs[0] = y0;
        compute();
    }

    void compute() const override {
        if (prev_x != (*ports->inputs[0] != 0))
            ports->outputs[0] = ports->outputs[0] == 0;
        prev_x = *ports->inputs[0] != 0;
    }

    std::string printMemory() const override {
        REGISTER_VAR(prev_x)
        std::stringstream line;
        line << "bool " << var_prev_x << " = false;";
        return line.str();
    }

    std::string printInit() const override {
        const auto t = CODE_NAME_IN(ports, 0);
        const auto y = CODE_NAME_OUT(ports, 0);
        REGISTER_VAR(prev_x)

        std::stringstream line;
        line << var_prev_x << " = " << t << " != 0;\n";
        line << y << " = " << y0 << ";\n";
        return line.str() + printSource();
    }

    types::string printSource() const override {
        const auto t = CODE_NAME_IN(ports, 0);
        const auto y = CODE_NAME_OUT(ports, 0);
        REGISTER_VAR(prev_x)

        std::stringstream line;
        line << "if (" << var_prev_x << " != (" << t << " != 0))\n";
        line << "    " << y << " = " << y << " == 0;\n";
        line << var_prev_x << " = " << t << " != 0;";
        return line.str();
    }
};
}