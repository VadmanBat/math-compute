#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <sstream>

namespace nrcki {
class ExtInSignal final : public Block {
    using Type = types::real;

    Ports<Type>* ports;
    Signals<Type>* signals;
    const Type y0;

public:
    explicit ExtInSignal(const Context& context, const Type& y0) :
        Block(context),
        y0(y0) {
        register_ports(ports = new Ports<Type>(0, 1));
        register_signals(signals = new Signals<Type>(1, 0));
    }

    void init() const override {
        ports->outputs[0] = y0;
    }

    void compute() const override {
        ports->outputs[0] = signals->inputs[0];
    }

    types::string printInit() const override {
        const auto y = CODE_NAME_OUT(ports, 0);

        std::stringstream line;
        line << y << " = " << y0 << ';';
        return line.str();
    }
};
}