#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <sstream>

namespace nrcki {
class Saturation final : public Block {
    using Type = types::real;

    Ports<Type>* ports;

    const Type x1, x2, y1, y2;
    const Type k, b;

public:
    explicit Saturation(const Context& context, const Type& x1, const Type& x2, const Type& y1, const Type& y2) :
        Block(context),
        x1(x1), x2(x2), y1(y1), y2(y2),
        k((y2 - y1) / (x2 - x1)),
        b(y1 - x1 * k) {
        ports = new Ports<Type>(1, 1);
        register_ports(ports);
    }

    void compute() const override {
        const auto x      = *ports->inputs[0];
        ports->outputs[0] = x < x1 ? y1 : x > x2 ? y2 : k * x + b;
        //ports->outputs[0] = x < x1 ? y1 : x > x2 ? y2 : std::fma(k, x, k);
    }

    types::string printSource() const override {
        const auto x = CODE_NAME_IN(ports, 0);
        const auto y = CODE_NAME_OUT(ports, 0);

        std::stringstream line;
        line << y << " = "
            << x << " < " << x1 << " ? " << y1 << " : "
            << x << " > " << x2 << " ? " << y2 << " : "
            << k << " * " << x << " + " << b << ';';
        return line.str();
    }
};
}