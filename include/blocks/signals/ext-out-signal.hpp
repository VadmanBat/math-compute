#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <sstream>

namespace nrcki {
class ExtOutSignal final : public Block {
    using Type = types::real;

    Ports<Type>* ports;

public:
    explicit ExtOutSignal(const Context& context) :
        Block(context) {
        ports = new Ports<Type>(1, 1);
        register_ports(ports);
    }

    void compute() const override {
        ports->outputs[0] = *ports->inputs[0];
    }

    types::string printInit() const override {
        const auto x = CODE_NAME_IN(ports, 0);
        const auto y = CODE_NAME_OUT(ports, 0);

        std::stringstream line;
        line << y << " = " << x << ';';
        return line.str();
    }
};
}