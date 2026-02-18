#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <sstream>

namespace nrcki {
class TTriggerL final : public Block {
    using Type = types::real;

    Ports<Type>* ports;

    const bool y0;

public:
    explicit TTriggerL(const Context& context, const bool y0 = false) :
        Block(context),
        y0(y0) {
        ports = new Ports<Type>(1, 1);
        register_ports(ports);
    }

    void init() const override {
        ports->outputs[0] = y0;
        compute();
    }

    void compute() const override {
        if (*ports->inputs[1] != 0)
            ports->outputs[0] = ports->outputs[0] == 0;
    }

    std::string printInit() const override {
        const auto y = CODE_NAME_OUT(ports, 0);

        std::stringstream line;
        line << y << " = " << y0 << ";\n";
        return line.str() + printSource();
    }

    types::string printSource() const override {
        const auto t = CODE_NAME_IN(ports, 0);
        const auto y = CODE_NAME_OUT(ports, 0);

        std::stringstream line;
        line << "f (" << t << " != 0)\n";
        line << "    " << y << " = " << y << " == 0;\n";
        return line.str();
    }
};
}