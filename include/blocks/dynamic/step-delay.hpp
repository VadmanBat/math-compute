#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <sstream>

namespace nrcki {
class StepDelay final : public Block {
    using Type = types::real;

    Ports<Type>* ports;
    const Type y0;
    mutable Type prev_x = 0;

public:
    explicit StepDelay(const Context& context, const Type& y0) :
        Block(context),
        y0(y0) {
        ports = new Ports<Type>(1, 1);
        register_ports(ports);

        //toggleFlag(Block::FlagType::CAN_UNTIE_LOOP);
    }

    void init() const override {
        ports->outputs[0] = y0;
        prev_x            = *ports->inputs[0];
    }

    void compute() const override {
        ports->outputs[0] = prev_x;
        prev_x            = *ports->inputs[0];
    }

    std::string printMemory() const override {
        const auto type = CODE_NAME_TYPE(Type);
        REGISTER_VAR(prev_x)

        std::stringstream line;
        line << type << ' ' << var_prev_x << ';';
        return line.str();
    }

    std::string printInit() const override {
        const auto x = CODE_NAME_IN(ports, 0);
        const auto y = CODE_NAME_OUT(ports, 0);
        REGISTER_VAR(prev_x)

        std::stringstream line;
        line << y << " = " << y0 << ";\n";
        line << var_prev_x << " = " << x << ';';
        return line.str();
    }

    types::string printSource() const override {
        const auto x = CODE_NAME_IN(ports, 0);
        const auto y = CODE_NAME_OUT(ports, 0);
        REGISTER_VAR(prev_x)

        std::stringstream line;
        line << y << " = " << var_prev_x << ";\n";
        line << var_prev_x << " = " << x << ';';
        return line.str();
    }
};
}