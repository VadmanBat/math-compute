#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <sstream>

namespace nrcki {
class Oscillatory final : public Block {
    using Type = types::real;

    Ports<Type>* ports;
    const Type k, T, b, y0, dy0;
    mutable Type dy, prev_y;

public:
    explicit Oscillatory(const Context& context,
                         const Type& k,
                         const Type& T,
                         const Type& b,
                         const Type& y0,
                         const Type& dy0)
        : Block(context),
          k(k), T(T), b(b), y0(y0), dy0(dy0),
          dy(dy0), prev_y(y0) {
        ports = new Ports<Type>(1, 1);
        register_ports(ports);

        toggleFlag(CAN_UNTIE_LOOP);
        toggleFlag(IMPLICIT_COMPUTE);
    }

    void init() const override {
        ports->outputs[0] = y0;
    }

    void compute() const override {
        prev_y            = ports->outputs[0];
        ports->outputs[0] += dy * Context::dt_sec;
        dy                += (k * *ports->inputs[0] - 2 * b * T * dy - prev_y) / (T * T) * Context::dt_sec;
    }

    std::string printMemory() const override {
        const auto type = CODE_NAME_TYPE(Type);
        REGISTER_VAR(dy)
        REGISTER_VAR(prev_y)

        std::stringstream line;
        line << type << ' ' << var_dy << ", " << var_prev_y << ';';
        return line.str();
    }

    std::string printInit() const override {
        const auto y = CODE_NAME_OUT(ports, 0);
        REGISTER_VAR(dy)

        std::stringstream line;
        line << y << " = " << y0 << ";\n";
        line << var_dy << " = " << dy0 << ';';
        return line.str();
    }

    types::string printSource() const override {
        const auto x = CODE_NAME_IN(ports, 0);
        const auto y = CODE_NAME_OUT(ports, 0);
        REGISTER_VAR(dy)
        REGISTER_VAR(prev_y)

        std::stringstream line;
        line << var_prev_y << " = " << y << ";\n";
        line << y << " += " << var_dy << " * dt_sec;\n";
        line << var_dy << " += (" << k << " * " << x << " - " << 2 * b * T << " * " << var_dy << " - " << var_prev_y <<
            ") / "
            << T * T << " * dt_sec;";
        return line.str();
    }
};
}