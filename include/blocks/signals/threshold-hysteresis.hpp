#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <sstream>

namespace nrcki {
class ThresholdHysteresis final : public Block {
    using Type = types::real;

    Ports<Type>* ports;

    const Type x1, x2;

public:
    explicit ThresholdHysteresis(const Context& context, const Type& x1, const Type& x2) :
        Block(context),
        x1(x1), x2(x2) {
        ports = new Ports<Type>(2, 2);
        register_ports(ports);
    }

    void init() const override {
        ports->outputs[0] = 0;
        compute();
    }

    void compute() const override {
        if (*ports->inputs[1] == 0) {
            ports->outputs[0] = 0;
            ports->outputs[1] = 0;
            return;
        }

        const auto x = *ports->inputs[0];
        if (x <= x1)
            ports->outputs[0] = 0;
        else if (x >= x2)
            ports->outputs[0] = 1;
        ports->outputs[1] = ports->outputs[0] == 0;
    }

    types::string printInit() const override {
        const auto y = CODE_NAME_OUT(ports, 0);

        std::stringstream line;
        line << y << " = 0;\n";
        return line.str() + printSource();
    }

    types::string printSource() const override {
        const auto x   = CODE_NAME_IN(ports, 0);
        const auto GSG = CODE_NAME_IN(ports, 1);
        const auto y1  = CODE_NAME_OUT(ports, 0);
        const auto y2  = CODE_NAME_OUT(ports, 1);

        std::stringstream line;
        line << "if (" << GSG << " == 0) {\n";
        line << "    " << y1 << " = 0;\n";
        line << "    " << y2 << " = 0;\n";
        line << "}\n";
        line << "else {\n";
        line << "    if (" << x << " <= " << x1 << ") {\n";
        line << "        " << y1 << " = 1;\n";
        line << "    else if (" << x << " >= " << x2 << ")\n";
        line << "        " << y1 << " = 0;";
        line << "    " << y2 << " = " << y1 << " == 0;\n";
        line << "}\n";
        return line.str();
    }
};
}