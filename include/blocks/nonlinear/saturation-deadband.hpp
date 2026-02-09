#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <sstream>

namespace nrcki {
class SaturationDeadband final : public Block {
    using Type = types::real;

    Ports<Type>* ports;

    const Type x1, x2, y1, y2;
    const Type db_x1, db_x2;

    const Type k1, k2, b1, b2;

public:
    explicit SaturationDeadband(const Context& context,
                                const Type& x1, const Type& x2,
                                const Type& y1, const Type& y2,
                                const Type& db_x1, const Type& db_x2) :
        Block(context),
        x1(x1), x2(x2), y1(y1), y2(y2),
        db_x1(db_x1), db_x2(db_x2),
        k1(y1 / (x1 - db_x1)),
        k2(y2 / (x2 - db_x2)),
        b1(y1 - k1 * x1),
        b2(y2 - k2 * x2) {
        ports = new Ports<Type>(1, 1);
        register_ports(ports);
    }

    void compute() const override {
        const auto x      = *ports->inputs[0];
        ports->outputs[0] = x < x1 ? y1 : x > x2 ? y2 : x < db_x1 ? k1 * x + b1 : x > db_x2 ? k2 * x + b2 : 0;
    }

    types::string printSource() const override {
        const auto x = CODE_NAME_IN(ports, 0);
        const auto y = CODE_NAME_OUT(ports, 0);

        std::stringstream line;
        line << y << " = "
            << x << " < " << x1 << " ? " << y1 << " : "
            << x << " > " << x2 << " ? " << y2 << " : "
            << x << " < " << db_x1 << " ? " << k1 << " * " << x << " + " << b1 << " : "
            << x << " < " << db_x2 << " ? " << k2 << " * " << x << " + " << b2 << " : 0;";
        return line.str();
    }
};
}