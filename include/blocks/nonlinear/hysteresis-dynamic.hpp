#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <sstream>

namespace nrcki {
class HysteresisDynamic : public Block {
    using Type = types::real;

    Ports<Type>* ports;

    const Type y1, y2;
    const bool y0;

public:
    explicit HysteresisDynamic(const Context& context,
                               const Type& y1, const Type& y2,
                               bool y0 = false) :
        Block(context),
        y1(y1), y2(y2), y0(y0) {
        ports = new Ports<Type>(3, 1);
        register_ports(ports);
    }

    void init() const override {
        ports->outputs[0] = y0 ? y2 : y1;
        compute();
    }

    void compute() const override {
        const auto x = *ports->inputs[0];
        if (x <= *ports->inputs[1])
            ports->outputs[0] = y1;
        else if (x >= *ports->inputs[2])
            ports->outputs[0] = y2;
    }

    types::string printInit() const override {
        const auto y = CODE_NAME_OUT(ports, 0);

        std::stringstream line;
        line << y << " = " << (y0 ? y2 : y1) << ";\n";
        return line.str() + printSource();
    }

    types::string printSource() const override {
        const auto x  = CODE_NAME_IN(ports, 0);
        const auto x1 = CODE_NAME_IN(ports, 1);
        const auto x2 = CODE_NAME_IN(ports, 2);
        const auto y  = CODE_NAME_OUT(ports, 0);

        std::stringstream line;
        line << "if (" << x << " <= " << x1 << ")\n";
        line << "    " << y << " = " << y1 << ";\n";
        line << "else if (" << x << " >= " << x2 << ")\n";
        line << "    " << y << " = " << y2 << ';';

        return line.str();
    }
};
}