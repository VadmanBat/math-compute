#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <sstream>

namespace nrcki {
class Xor final : public Block {
    using Type = types::real;
    using size = types::size;

    Ports<Type>* ports;

public:
    explicit Xor(const Context& context, size n) :
        Block(context) {
        ports = new Ports<Type>(n, 1);
        register_ports(ports);
    }

    void compute() const override {
        bool res = *ports->inputs[0] != 0;
        for (size i = 1; i < ports->input_count; ++i)
            res ^= *ports->inputs[i] != 0;
        ports->outputs[0] = res;
    }

    types::string printSource() const override {
        auto x       = CODE_NAME_IN(ports, 0);
        const auto y = CODE_NAME_OUT(ports, 0);

        std::stringstream line;
        line << y << " = (" << x << " != 0)";
        for (size i = 1; i < ports->input_count; ++i) {
            x = CODE_NAME_IN(ports, i);
            line << " ^ (" << x << " != 0)";
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