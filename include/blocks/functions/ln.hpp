#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <sstream>
#include <cmath>

namespace nrcki {
class Ln final : public Block {
    using Type = types::real;

    Ports<Type>* ports;

public:
    explicit Ln(const Context& context) :
        Block(context) {
        ports = new Ports<Type>(1, 1);
        register_ports(ports);
    }

    void compute() const override {
        ports->outputs[0] = std::log(*ports->inputs[0]);
    }

    types::string printSource() const override {
        const auto x = CODE_NAME_IN(ports, 0);
        const auto y = CODE_NAME_OUT(ports, 0);

        std::stringstream line;
        line << y << " =  sqrt(" << x << ");\n";
        return line.str();
    }
};
}