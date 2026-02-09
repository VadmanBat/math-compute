#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <sstream>

namespace nrcki {
class RisingPulse final : public Block {
    using Type = types::real;

    Ports<Type>* ports;

    mutable bool prev_x = false;

public:
    explicit RisingPulse(const Context& context) :
        Block(context) {
        ports = new Ports<Type>(1, 1);
        register_ports(ports);
    }

    void init() const override {
        ports->outputs[0] = false;
        prev_x            = *ports->inputs[0] != 0;
    }

    void compute() const override {
        ports->outputs[0] = !prev_x && *ports->inputs[0] != 0;
        prev_x            = *ports->inputs[0] != 0;
    }

    std::string printMemory() const override {
        REGISTER_VAR(prev_x)

        std::stringstream line;
        line << "bool " << var_prev_x << " = false;";
        return line.str();
    }

    std::string printInit() const override {
        const auto x = CODE_NAME_IN(ports, 0);
        const auto y = CODE_NAME_OUT(ports, 0);
        REGISTER_VAR(prev_x)

        std::stringstream line;
        line << y << " = false;\n";
        line << var_prev_x << " = " << x << " != 0;";
        return line.str();
    }

    types::string printSource() const override {
        const auto x = CODE_NAME_IN(ports, 0);
        const auto y = CODE_NAME_OUT(ports, 0);
        REGISTER_VAR(prev_x)

        std::stringstream line;
        line << y << " = !" << var_prev_x << " && " << x << " != 0;\n";
        line << var_prev_x << " = " << x << " != 0;";
        return line.str();
    }
};
}