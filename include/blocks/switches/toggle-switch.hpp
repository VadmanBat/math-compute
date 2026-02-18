#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <sstream>

namespace nrcki {
class ToggleSwitch final : public Block {
    using Type = types::real;

    Ports<Type>* ports;

public:
    explicit ToggleSwitch(const Context& context) :
        Block(context) {
        ports = new Ports<Type>(3, 1);
        register_ports(ports);
    }

    void compute() const override {
        ports->outputs[0] = *ports->inputs[2] != 0 ? *ports->inputs[1] : *ports->inputs[0];
    }

    types::string printSource() const override {
        const auto x1 = CODE_NAME_IN(ports, 0);
        const auto x2 = CODE_NAME_IN(ports, 1);
        const auto x3 = CODE_NAME_IN(ports, 2);
        const auto y  = CODE_NAME_OUT(ports, 0);

        std::stringstream line;
        line << y << " = " << x3 << " != 0 ? " << x2 << " : " << x1 << ';';
        return line.str();
    }
};
}