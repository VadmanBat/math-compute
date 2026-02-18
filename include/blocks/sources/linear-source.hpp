#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <sstream>

namespace nrcki {
class LinearSource final : public Block {
    using Type = types::real;

    Ports<Type>* ports;
    const Type k, b;

public:
    explicit LinearSource(const Context& context, const Type& k, const Type& b) :
        Block(context),
        k(k), b(b) {
        ports = new Ports<Type>(0, 1);
        register_ports(ports);
    }

    void compute() const override {
        ports->outputs[0] = b + k * static_cast<Type>(context.time) / Context::sec;
    }

    types::string printSource() const override {
        const auto y = CODE_NAME_OUT(ports, 0);
        std::stringstream line;
        line << y << " = (" << k << ") * time / sec + " << b << ';';
        return line.str();
    }
};
}