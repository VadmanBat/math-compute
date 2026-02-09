#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <sstream>

namespace nrcki {
class Hysteresis final : public Block {
    using Type = types::real;

    Ports<Type>* ports;

    const bool y0;
    const Type x1, x2, y1, y2;

public:
    explicit Hysteresis(const Context& context,
                        const Type& x1, const Type& x2,
                        const Type& y1, const Type& y2,
                        bool y0 = false) :
        Block(context),
        x1(x1), x2(x2), y1(y1), y2(y2), y0(y0) {
        ports = new Ports<Type>(1, 1);
        register_ports(ports);
    }

    void init() const override {
        ports->outputs[0] = y0 ? y2 : y1;
        compute();
    }

    void compute() const override {
        const auto x = *ports->inputs[0];
        if (x <= x1)
            ports->outputs[0] = y1;
        else if (x >= x2)
            ports->outputs[0] = y2;
    }

    types::string printInit() const override {
        const auto y = CODE_NAME_OUT(ports, 0);

        std::stringstream line;
        line << y << " = " << (y0 ? y2 : y1) << ";\n";
        return line.str() + printSource();
    }

    types::string printSource() const override {
        const auto x = CODE_NAME_IN(ports, 0);
        const auto y = CODE_NAME_OUT(ports, 0);

        std::stringstream line;
        if (x1 != x2) {
            line << "if (" << x << " <= " << x1 << ") {\n";
            line << "    " << y << " = " << y1 << ";\n";
            line << "else if (" << x << " >= " << x2 << ")\n";
            line << "    " << y << " = " << y2 << ';';
        }
        else
            line << y << " = " << x << " <= " << x1 << " ? " << y1 << " : " << y2 << ';';
        return line.str();
    }
};
}