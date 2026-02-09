#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <sstream>

namespace nrcki {
class IntInSignal final : public Block {
    using Type = types::real;

    Ports<Type>* ports;
    const Type y0;
    const uint16_t out_block;
    mutable Type* in = nullptr;

public:
    explicit IntInSignal(const Context& context, const Type& y0, uint16_t out_block) :
        Block(context),
        y0(y0),
        out_block(out_block) {
        ports = new Ports<Type>(0, 1);
        register_ports(ports);
    }

    void init() const override {
        in                = static_cast<Type*>(context.blocks[out_block]->getOutputPortAbsolute(0));
        ports->outputs[0] = y0;
    }

    void compute() const override {
        ports->outputs[0] = *in;
    }

    types::string printInit() const override {
        const auto y = CODE_NAME_OUT(ports, 0);

        std::stringstream line;
        line << y << " = " << y0 << ';';
        return line.str();
    }

    types::string printSource() const override {
        const auto x = types::type_codename(ports->getTypeHash()) + '[' + std::to_string(
                           context.ports_info.at(in).relative_index) + ']';
        const auto y = CODE_NAME_OUT(ports, 0);

        std::stringstream line;
        line << y << " = " << x << ';';
        return line.str();
    }
};
}