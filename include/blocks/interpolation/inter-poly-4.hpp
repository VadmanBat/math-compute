#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <sstream>

namespace nrcki {
class InterPoly4 final : public Block {
    using Type = types::real;
    using Vec  = std::vector<Type>;

    Ports<Type>* ports;

    const Type C[5];
    const Type x1, dx;

public:
    explicit InterPoly4(const Context& context, const Type& x1, const Type& x2, const Vec& y) :
        Block(context),
        C(y[2],
          4 * (y[3] - y[1]) / 3 - (y[4] - y[0]) / 6,
          -5 * y[2] + 8 * (y[3] + y[1]) / 3 - (y[0] + y[4]) / 6,
          2 * (y[4] - y[0]) / 3 - 4 * (y[3] - y[1]) / 3,
          2 * y[3] + 2 * (y[0] + y[4]) / 3 - 8 * (y[3] + y[1]) / 3
            ),
        x1(x1), dx(x2 - x1) {
        ports = new Ports<Type>(1, 1);
        register_ports(ports);
    }

    void compute() const override {
        const auto x      = 2 * (*ports->inputs[0] - x1) / dx - 1;
        ports->outputs[0] = ((((C[4] * x) + C[3]) * x + C[2]) * x + C[1]) * x + C[0];
    }

    std::string printMemory() const override {
        const auto type = CODE_NAME_TYPE(Type);
        REGISTER_VAR(x_in)

        std::stringstream line;
        line << type << ' ' << var_x_in << ';';
        return line.str();
    }

    types::string printSource() const override {
        const auto x = CODE_NAME_IN(ports, 0);
        const auto y = CODE_NAME_OUT(ports, 0);
        REGISTER_VAR(x_in)

        std::stringstream line;
        line << var_x_in << " = 2 * (" << x << " - (" << x1 << ")) / (" << dx << ") - 1;";
        line << y << " = ((((("
            << C[4] << ") * " << var_x_in << " + ("
            << C[3] << ")) * " << var_x_in << " + ("
            << C[2] << ")) * " << var_x_in << " + ("
            << C[1] << ")) * " << var_x_in << " + ("
            << C[0] << ");";
        return line.str();
    }
};