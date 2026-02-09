#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <sstream>

namespace nrcki {
class Deadband final : public Block {
    using Type = types::real;

    Ports<Type>* ports;

    const Type x1, x2, k;

public:
    explicit Deadband(const Context& context, const Type& x1, const Type& x2, const Type& k) :
        Block(context),
        x1(x1), x2(x2), k(k) {
        ports = new Ports<Type>(1, 1);
        register_ports(ports);
    }

    void compute() const override {
        const auto x      = *ports->inputs[0];
        ports->outputs[0] = x < x1 ? k * (x - x1) : x > x2 ? k * (x - x2) : 0;
    }

    types::string printSource() const override {
        const auto x = CODE_NAME_IN(ports, 0);
        const auto y = CODE_NAME_OUT(ports, 0);

        std::stringstream line;
        line << y << " = "
            << x << " < " << x1 << " ? " << k << " * (" << x << " - " << x1 << ") : "
            << x << " > " << x2 << " ? " << k << " * (" << x << " - " << x2 << ") : 0;";
        return line.str();
    }
};
}