#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <sstream>

namespace nrcki {
class Integrator final : public Block {
    using Type = types::real;

    Ports<Type>* ports;
    const Type k, y0;

public:
    explicit Integrator(const Context& context, const Type& k, const Type& y0) :
        Block(context),
        k(k), y0(y0) {
        ports = new Ports<Type>(1, 1);
        register_ports(ports);

        toggleFlag(CAN_UNTIE_LOOP);
        toggleFlag(IMPLICIT_COMPUTE);
    }

    void init() const override {
        ports->outputs[0] = y0;
    }

    void compute() const override {
        ports->outputs[0] += k * (*ports->inputs[0]) * Context::dt_sec;
    }

    std::string printInit() const override {
        const auto y = CODE_NAME_OUT(ports, 0);

        std::stringstream line;
        line << y << " = " << y0 << ';';
        return line.str();
    }

    types::string printSource() const override {
        const auto x = CODE_NAME_IN(ports, 0);
        const auto y = CODE_NAME_OUT(ports, 0);

        std::stringstream line;
        line << y << " += " << k << " * " << x << " * dt_sec;";
        return line.str();
    }
};
}