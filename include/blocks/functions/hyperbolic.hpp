#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <sstream>

namespace nrcki {
class Hyperbolic final : public Block {
    using Type = types::real;

    Ports<Type>* ports;
    const Type k;

public:
    explicit Hyperbolic(const Context& context, const Type& k) :
        Block(context),
        k(k) {
        ports = new Ports<Type>(1, 1);
        register_ports(ports);
    }

    void compute() const override {
        ports->outputs[0] = k / *ports->inputs[0];
    }

    types::string printSource() const override {
        const auto x = CODE_NAME_IN(ports, 0);
        const auto y = CODE_NAME_OUT(ports, 0);

        std::stringstream line;
        line << y << " = " << k << " / " << x << ";\n";
        return line.str();
    }
};
}