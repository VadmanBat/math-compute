#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <algorithm>
#include <sstream>

namespace nrcki {
class Min4 final : public Block {
    using Type = types::real;

    Ports<Type>* ports;

public:
    explicit Min4(const Context& context) :
        Block(context) {
        ports = new Ports<Type>(8, 2);
        register_ports(ports);
    }

    void compute() const override {
        ports->outputs[0] = std::min({
            *ports->inputs[4] != 0 ? *ports->inputs[0] : +9'999'999,
            *ports->inputs[5] != 0 ? *ports->inputs[1] : +9'999'999,
            *ports->inputs[6] != 0 ? *ports->inputs[2] : +9'999'999,
            *ports->inputs[7] != 0 ? *ports->inputs[3] : +9'999'999,
        });
        ports->outputs[1] = ports->outputs[0] != +9'999'999;
    }

    types::string printSource() const override {
        const auto x1 = CODE_NAME_IN(ports, 0);
        const auto x2 = CODE_NAME_IN(ports, 1);
        const auto x3 = CODE_NAME_IN(ports, 2);
        const auto x4 = CODE_NAME_IN(ports, 3);
        const auto g1 = CODE_NAME_IN(ports, 4);
        const auto g2 = CODE_NAME_IN(ports, 5);
        const auto g3 = CODE_NAME_IN(ports, 6);
        const auto g4 = CODE_NAME_IN(ports, 7);

        const auto y1 = CODE_NAME_OUT(ports, 0);
        const auto y2 = CODE_NAME_OUT(ports, 1);

        std::stringstream line;
        line << y1 << " = min({\n";
        line << "    " << g1 << " != 0 ? " << x1 << " : +9'999'999,\n";
        line << "    " << g2 << " != 0 ? " << x2 << " : +9'999'999,\n";
        line << "    " << g3 << " != 0 ? " << x3 << " : +9'999'999,\n";
        line << "    " << g4 << " != 0 ? " << x4 << " : +9'999'999,\n";
        line << "})\n";
        line << y2 << " = " << y1 << " != +9'999'999;";
        return line.str();
    }
};
}