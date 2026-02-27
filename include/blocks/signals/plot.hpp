#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

namespace nrcki {
class Plot final : public Block {
    using Type = types::real;

    Ports<Type>* ports;

public:
    explicit Plot(const Context& context, const uint8_t count) :
        Block(context) {
        ports = new Ports<Type>(count, 0);
        register_ports(ports);

        toggleFlag(Block::FlagType::CONSTANT);
    }
};
}