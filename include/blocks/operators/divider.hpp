#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <sstream>

namespace nrcki {
class Divider final : public Block {
    using Type = types::real;
    using size = types::size;

    Ports<Type>* ports;
    const Type value_if_div_null;

public:
    explicit Divider(const Context& context, const Type& value_if_div_null) :
        Block(context),
        value_if_div_null(value_if_div_null) {
        ports = new Ports<Type>(2, 1);
        register_ports(ports);
    }

    void compute() const override {
        ports->outputs[0] = *ports->inputs[1] != 0 ? *ports->inputs[0] / *ports->inputs[1] : value_if_div_null;
    }

    types::string printSource() const override {
        const auto x1 = CODE_NAME_IN(ports, 0);
        const auto x2 = CODE_NAME_IN(ports, 1);
        const auto y  = CODE_NAME_OUT(ports, 0);

        std::stringstream line;
        line << y << " = " << x2 << " != 0 ? " << x1 << " / " << x2 << " : " << value_if_div_null << ';';
        return line.str();
    }

    bool tryMakeConstant() override {
        if (context.ports_info[ports->inputs[0]].is_constant && context.ports_info[ports->inputs[1]].is_constant) {
            context.ports_info[&ports->outputs[0]].is_constant = true;
            toggleFlag(Block::FlagType::CONSTANT);
            return true;
        }
        return false;
    }
};
}