#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <sstream>

namespace nrcki {
class ExtInSignal final : public Block {
    using Type = types::real;

    Ports<Type>* ports;
    const Type y0;

public:
    explicit ExtInSignal(const Context& context, const Type& y0) :
        Block(context),
        y0(y0) {
        ports = new Ports<Type>(0, 1);
        register_ports(ports);
    }

    void init() const override {
        ports->outputs[0] = y0;
    }

    void update(const Type& y) {
        ports->outputs[0] = y;
    }

    types::string printInit() const override {
        const auto y = CODE_NAME_OUT(ports, 0);

        std::stringstream line;
        line << y << " = " << y0 << ';';
        return line.str();
    }
};
}