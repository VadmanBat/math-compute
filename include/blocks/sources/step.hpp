#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <sstream>

namespace nrcki {
class Step final : public Block {
    using Type = types::real;
    using Time = types::time;

    Ports<Type>* ports;
    const Type y0, value;
    Time time;

public:
    explicit Step(const Context& context, const Type& time, const Type& value, const Type& y0) :
        Block(context),
        time(static_cast<Time>(time * Context::sec)),
        value(value), y0(y0) {
        ports = new Ports<Type>(0, 1);
        register_ports(ports);
    }

    void compute() const override {
        ports->outputs[0] = context.time < time ? y0 : value;
    }

    types::string printSource() const override {
        const auto y = CODE_NAME_OUT(ports, 0);

        std::stringstream line;
        line << y << " = time < " << time << " ? " << y0 << " : " << value << ';';
        return line.str();
    }
};
}