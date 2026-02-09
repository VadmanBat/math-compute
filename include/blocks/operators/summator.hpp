#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <sstream>

namespace nrcki {
class Summator final : public Block {
    using Type = types::real;
    using size = types::size;

    Ports<Type>* ports;
    const std::vector<Type> coeffs;

public:
    explicit Summator(const Context& context, const std::vector<Type>& coefficients) :
        Block(context),
        coeffs(coefficients) {
        ports = new Ports<Type>(coefficients.size(), 1);
        register_ports(ports);
    }

    void compute() const override {
        ports->outputs[0] = coeffs[0] * (*ports->inputs[0]);
        for (size i = 1; i < ports->input_count; ++i)
            ports->outputs[0] += coeffs[i] * (*ports->inputs[i]);
    }

    types::string printSource() const override {
        auto x       = CODE_NAME_IN(ports, 0);
        const auto y = CODE_NAME_OUT(ports, 0);

        std::stringstream line;
        line << y << " = (" << coeffs[0] << ") * " << x;
        for (size i = 1; i < ports->input_count; ++i) {
            x = CODE_NAME_IN(ports, i);
            line << " + (" << coeffs[i] << ") * " << x;
        }
        line << ';';
        return line.str();
    }

    bool tryMakeConstant() override {
        for (size i = 0; i < ports->input_count; ++i)
            if (!context.ports_info[ports->inputs[i]].is_constant)
                return false;
        context.ports_info[&ports->outputs[0]].is_constant = true;
        toggleFlag(Block::FlagType::CONSTANT);
        return true;
    }
};
}