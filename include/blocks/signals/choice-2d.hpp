#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <sstream>

namespace nrcki {
class Choice2D final : public Block {
    using Type = types::real;

    Ports<Type>* ports;

public:
    explicit Choice2D(const Context& context) :
        Block(context) {
        ports = new Ports<Type>(2, 3);
        register_ports(ports);
    }

    void compute() const override {
        ports->outputs[0] = (*ports->inputs[0] != 0) || (*ports->inputs[1] != 0);
        ports->outputs[1] = (*ports->inputs[0] != 0) && (*ports->inputs[1] != 0);
        ports->outputs[2] = (*ports->inputs[0] != 0) != (*ports->inputs[1] != 0);
    }

    types::string printSource() const override {
        const auto x1 = CODE_NAME_IN(ports, 0);
        const auto x2 = CODE_NAME_IN(ports, 1);

        const auto y1 = CODE_NAME_OUT(ports, 0);
        const auto y2 = CODE_NAME_OUT(ports, 1);
        const auto y3 = CODE_NAME_OUT(ports, 2);

        std::stringstream line;
        line << y1 << " = (" << x1 << " != 0) || (" << x2 << " != 0);\n";
        line << y2 << " = (" << x1 << " != 0) && (" << x2 << " != 0);\n";
        line << y3 << " = (" << x1 << " != 0) != (" << x2 << " != 0);";
        return line.str();
    }
};
}