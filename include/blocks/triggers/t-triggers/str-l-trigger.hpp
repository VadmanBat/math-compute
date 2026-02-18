#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <sstream>

namespace nrcki {
class StrLTrigger final : public Block {
    using Type = types::real;

    Ports<Type>* ports;

    const bool y0;

public:
    StrLTrigger(const Context& context, bool y0 = false) :
        Block(context),
        y0(y0) {
        ports = new Ports<Type>(3, 1);
        register_ports(ports);
    }

    void init() const override {
        ports->outputs[0] = y0;
        compute();
    }

    void compute() const override {
        if (*ports->inputs[0] != 0)
            ports->outputs[0] = true;
        else if (*ports->inputs[2] != 0)
            ports->outputs[0] = false;
        else if (*ports->inputs[1] != 0)
            ports->outputs[0] = ports->outputs[0] == 0;
    }

    std::string printInit() const override {
        const auto y = CODE_NAME_OUT(ports, 0);

        std::stringstream line;
        line << y << " = " << y0 << ";\n";
        return line.str() + printSource();
    }

    types::string printSource() const override {
        const auto s = CODE_NAME_IN(ports, 0);
        const auto t = CODE_NAME_IN(ports, 1);
        const auto r = CODE_NAME_IN(ports, 2);
        const auto y = CODE_NAME_OUT(ports, 0);

        std::stringstream line;
        line << "if (" << s << " != 0)\n";
        line << "    " << y << " = true;\n";
        line << "else if (" << r << " != 0)\n";
        line << "    " << y << " = false;\n";
        line << "else if (" << t << " != 0)\n";
        line << "    " << y << " = " << y << " == 0;";
        return line.str();
    }
};
}