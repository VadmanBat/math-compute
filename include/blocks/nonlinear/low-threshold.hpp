#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <sstream>

namespace nrcki {
class LowThreshold final : public Block {
    using Type = types::real;

    Ports<Type>* ports;

    const Type activation, deactivation;

public:
    explicit LowThreshold(const Context& context, const Type& activation_threshold, const Type& deactivation_delta) :
        Block(context),
        activation(activation_threshold),
        deactivation(activation_threshold + deactivation_delta) {
        ports = new Ports<Type>(1, 1);
        register_ports(ports);
    }

    void init() const override {
        ports->outputs[0] = *ports->inputs[0] < activation;
    }

    void compute() const override {
        const auto input = *ports->inputs[0];
        if (input > deactivation)
            ports->outputs[0] = false;
        else if (input < activation)
            ports->outputs[0] = true;
    }

    types::string printInit() const override {
        const auto x = CODE_NAME_IN(ports, 0);
        const auto y = CODE_NAME_OUT(ports, 0);

        std::stringstream line;
        line << y << " = " << x << " < " << activation << ';';
        return line.str();
    }

    types::string printSource() const override {
        const auto x = CODE_NAME_IN(ports, 0);
        const auto y = CODE_NAME_OUT(ports, 0);

        std::stringstream line;
        if (activation != deactivation) {
            line << "if (" << x << " > " << deactivation << ") {\n";
            line << "    " << y << " = false;\n";
            line << "else if (" << x << " < " << activation << ")\n";
            line << "    " << y << " = true;";
        }
        else
            line << y << " = " << x << " < " << activation << ';';;
        return line.str();
    }
};
}