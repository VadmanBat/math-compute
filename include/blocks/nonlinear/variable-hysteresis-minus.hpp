#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <sstream>

namespace nrcki {
class VariableHysteresisMinus final : public Block {
    using Type = types::real;

    Ports<Type>* ports;
    mutable Type prev_y = 0;

public:
    explicit VariableHysteresisMinus(const Context& context) :
        Block(context) {
        ports = new Ports<Type>(3, 1);
        register_ports(ports);
    }

    void init() const override {
        prev_y            = 0;
        ports->outputs[0] = 0;
    }

    void compute() const override {
        auto x       = *ports->inputs[0];
        const auto d = *ports->inputs[1];
        const auto r = *ports->inputs[2];

        x                 = 100 - d + r * prev_y - x;
        prev_y            = ports->outputs[0];
        ports->outputs[0] = x <= -100 ? -1 : x >= 100 ? 1 : 0;
    }

    types::string printInit() const override {
        const auto y = CODE_NAME_OUT(ports, 0);
        REGISTER_VAR(prev_y)

        std::stringstream line;
        line << y << " = 0;";
        line << var_prev_y << " = 0;";
        return line.str();
    }

    types::string printMemory() const override {
        const auto type = CODE_NAME_TYPE(Type);
        REGISTER_VAR(prev_y)

        std::stringstream line;
        line << type << ' ' << var_prev_y << " = 0;";
        return line.str();
    }

    types::string printSource() const override {
        const auto x    = CODE_NAME_IN(ports, 0);
        const auto d    = CODE_NAME_IN(ports, 1);
        const auto r    = CODE_NAME_IN(ports, 2);
        const auto y    = CODE_NAME_OUT(ports, 0);
        const auto type = CODE_NAME_TYPE(Type);
        REGISTER_VAR(prev_y)

        std::stringstream line;
        line << "{\n";
        line << "    " << type << " t = 100 - " << d << " + " << r << " * " << var_prev_y << " - " << x << ";\n";
        line << "    " << var_prev_y << " = " << y << ";\n";
        line << "    " << y << " = t <= -100 ? -1 : t >= 100 ? 1 : 0;\n";
        line << '}';
        return line.str();
    }
};
}