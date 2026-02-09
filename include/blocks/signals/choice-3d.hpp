#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <sstream>

namespace nrcki {
class Choice3D final : public Block {
    using Type = types::real;

    Ports<Type>* ports;

public:
    explicit Choice3D(const Context& context) :
        Block(context) {
        ports = new Ports<Type>(3, 4);
        register_ports(ports);
    }

    void compute() const override {
        const int count =
            (*ports->inputs[0] != 0) +
            (*ports->inputs[1] != 0) +
            (*ports->inputs[2] != 0);
        ports->outputs[0] = count >= 1;
        ports->outputs[1] = count >= 2;
        ports->outputs[2] = count == 3;
        ports->outputs[3] = count == 0 || count == 3;
    }

    types::string printSource() const override {
        const auto x1 = CODE_NAME_IN(ports, 0);
        const auto x2 = CODE_NAME_IN(ports, 1);
        const auto x3 = CODE_NAME_IN(ports, 2);

        const auto y1 = CODE_NAME_OUT(ports, 0);
        const auto y2 = CODE_NAME_OUT(ports, 1);
        const auto y3 = CODE_NAME_OUT(ports, 2);
        const auto y4 = CODE_NAME_OUT(ports, 3);

        std::stringstream line;
        line << "{\n";
        line << "    const int count = ("
            << x1 << " != 0) + ("
            << x2 << " != 0) + ("
            << x3 << " != 0)";
        line << "    " << y1 << " = count >= 1;\n";
        line << "    " << y2 << " = count >= 2;\n";
        line << "    " << y3 << " = count == 3;\n";
        line << "    " << y4 << " = count == 0 || count == 3;\n";
        line << "}";

        return line.str();
    }
};
}