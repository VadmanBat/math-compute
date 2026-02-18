#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <sstream>

namespace nrcki {
class RsTrigger final : public Block {
    using Type = types::real;

    Ports<Type>* ports;
    const Type y0;

public:
    explicit RsTrigger(const Context& context, const Type& y0) :
        Block(context),
        y0(y0) {
        ports = new Ports<Type>(2, 1);
        register_ports(ports);
    }

    void init() const override {
        ports->outputs[0] = y0;
        compute();
    }

    void compute() const override {
        if (*ports->inputs[1] != 0)
            ports->outputs[0] = false;
        else if (*ports->inputs[0] != 0)
            ports->outputs[0] = true;
    }

    std::string printInit() const override {
        const auto y = CODE_NAME_OUT(ports, 0);
        std::stringstream line;
        line << y << " = " << y0 << ";\n";
        return line.str() + printSource();
    }

    types::string printSource() const override {
        const auto s = CODE_NAME_IN(ports, 0);
        const auto r = CODE_NAME_IN(ports, 1);
        const auto y = CODE_NAME_OUT(ports, 0);

        std::stringstream line;
        line << "if (" << r << " != 0)\n";
        line << "    " << y << " = false;\n";
        line << "else if (" << s << " != 0)\n";
        line << "    " << y << " = true;";
        return line.str();
    }
};
}