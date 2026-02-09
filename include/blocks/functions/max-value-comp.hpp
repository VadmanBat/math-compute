#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <sstream>

namespace nrcki {
class MinValueComp final : public Block {
    using Type = types::real;

    Ports<Type>* ports;

public:
    explicit MinValueComp(const Context& context) :
        Block(context) {
        ports = new Ports<Type>(2, 2);
        register_ports(ports);
    }

    void compute() const override {
        ports->outputs[1] = *ports->inputs[0] > *ports->inputs[1];
        ports->outputs[0] = static_cast<bool>(ports->outputs[1]) ? *ports->inputs[0] : *ports->inputs[1];
    }

    types::string printSource() const override {
        const auto x1 = CODE_NAME_IN(ports, 0);
        const auto x2 = CODE_NAME_IN(ports, 1);
        const auto y1 = CODE_NAME_OUT(ports, 0);
        const auto y2 = CODE_NAME_OUT(ports, 1);

        std::stringstream line;
        line << y2 << " = " << x1 << " > " << x2 << ";\n";
        line << y1 << " = " << y2 << " ? " << x1 << " : " << x2 << ";\n";
        return line.str();
    }
};
}