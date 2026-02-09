#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <sstream>

namespace nrcki {
class InertialDifferential final : public Block {
    using Type = types::real;

    Ports<Type>* ports;
    const Type k, T, y0;
    mutable Type prev_x = 0;

public:
    explicit InertialDifferential(const Context& context, const Type& k, const Type& T, const Type y0) :
        Block(context),
        k(k), T(T), y0(y0) {
        ports = new Ports<Type>(1, 1);
        register_ports(ports);
    }

    void init() const override {
        ports->outputs[0] = y0;
        prev_x            = *ports->inputs[0];
    }

    void compute() const override {
        ports->outputs[0] += (k * (*ports->inputs[0] - prev_x) - ports->outputs[0] * Context::dt_sec) / T;
        prev_x            = *ports->inputs[0];
    }

    std::string printMemory() const override {
        const auto type = CODE_NAME_TYPE(Type);
        REGISTER_VAR(prev_x)

        std::stringstream line;
        line << type << ' ' << var_prev_x << ';';
        return line.str();
    }

    std::string printInit() const override {
        const auto x = CODE_NAME_IN(ports, 0);
        const auto y = CODE_NAME_OUT(ports, 0);
        REGISTER_VAR(prev_x)

        std::stringstream line;
        line << y << " = " << y0 << ";\n";
        line << var_prev_x << " = " << x << ';';
        return line.str();
    }

    types::string printSource() const override {
        const auto x = CODE_NAME_IN(ports, 0);
        const auto y = CODE_NAME_OUT(ports, 0);
        REGISTER_VAR(prev_x)

        std::stringstream line;
        line << y << " += (" << k << " * (" << x << " - " << var_prev_x << ") - " << y << " * dt_sec) / " << T << ";\n";
        line << var_prev_x << " = " << x << ';';
        return line.str();
    }
};
}