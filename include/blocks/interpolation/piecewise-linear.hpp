#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <sstream>
#include <utility>

#include "plc-code.hpp"

namespace nrcki {
class PiecewiseLinear final : public Block {
    using Type = types::real;
    using Vec  = std::vector<Type>;

    Ports<Type>* ports;

    const std::size_t n;
    const bool is_extra_bound;

    PiecewiseLinearCharacteristic bsc;

public:
    explicit PiecewiseLinear(const Context& context, const Vec& x, const Vec& y, bool is_extra_bound) :
        Block(context),
        bsc(x, y, is_extra_bound),
        n(x.size()), is_extra_bound(is_extra_bound) {
        ports = new Ports<Type>(1, 1);
        register_ports(ports);
    }

    void compute() const override {
        ports->outputs[0] = bsc(*ports->inputs[0]);
    }

    std::string printMemory() const override {
        REGISTER_VAR(bcs)
        std::stringstream line;
        line << "PiecewiseLinearCharacteristic " << var_bcs << " = {\n";
        line << "    " << bsc.getX() << ",\n";
        line << "    " << bsc.getY() << "\n";
        line << "};";
        return line.str();
    }

    types::string printSource() const override {
        const auto x = CODE_NAME_IN(ports, 0);
        const auto y = CODE_NAME_OUT(ports, 0);
        REGISTER_VAR(bcs)

        std::stringstream line;
        switch (2 * (n > 16) + is_extra_bound) {
            case 0:
                line << y << " = " << var_bcs << ".compute_SL(" << x << ");";
                break;
            case 1:
                line << y << " = " << var_bcs << ".compute_EL(" << x << ");";
                break;
            case 2:
                line << y << " = " << var_bcs << ".compute_SB(" << x << ");";
                break;
            case 3:
                line << y << " = " << var_bcs << ".compute_EB(" << x << ");";
                break;
        }
        return line.str();
    }
};
}