#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <sstream>

namespace nrcki {
class Constant final : public Block {
    using Type = types::real;

    Ports<Type>* ports;
    const Type value;

public:
    explicit Constant(const Context& context, const Type& value) :
        Block(context),
        value(value) {
        ports = new Ports<Type>(0, 1);
        register_ports(ports);

        toggleFlag(Block::FlagType::CONSTANT);
    }

    void init() const override {
        ports->outputs[0]                                  = value;
        context.ports_info[&ports->outputs[0]].is_constant = true;
    }

    types::string printInit() const override {
        const auto y = CODE_NAME_OUT(ports, 0);

        std::stringstream line;
        line << y << " = " << value << ';';
        return line.str();
    }
};
}