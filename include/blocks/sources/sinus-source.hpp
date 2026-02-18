#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <sstream>
#include <cmath>

namespace nrcki {
class SinusSource final : public Block {
    using Type = types::real;

    Ports<Type>* ports;
    const Type a, w, f; /// амплитуда, частота, фаза

public:
    explicit SinusSource(const Context& context, const Type& a, const Type& w, const Type& f) :
        Block(context),
        a(a), w(w), f(f) {
        ports = new Ports<Type>(0, 1);
        register_ports(ports);
    }

    void compute() const override {
        ports->outputs[0] = a * std::sin(w * static_cast<Type>(context.time) / Context::sec + f);
    }

    types::string printSource() const override {
        const auto y = CODE_NAME_OUT(ports, 0);

        std::stringstream line;
        line << y << " = " << a << " * sin(" << w << " * time / sec + (" << f << "));";
        return line.str();
    }
};
}