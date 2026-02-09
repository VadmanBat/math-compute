#pragma once

#include "constants/config.hxx"

#include <vector>
#include <algorithm>
#include <sstream>

namespace nrcki {
class [[maybe_unused]] PiecewiseLinearCharacteristic {
    using Type = types::real;
    using Vec = std::vector <Type>;

    using ComputeFunction = Type (PiecewiseLinearCharacteristic::*)(const Type&) const noexcept;

    const Vec args, values;
    const std::size_t n;
    const Type* x;
    const Type* y;

    const std::vector <std::pair <Type, Type>> kb_coeffs;
    const std::pair <Type, Type>* kb;

    const ComputeFunction compute;

    [[nodiscard]] inline std::vector <std::pair <Type, Type>> compute_kb() const noexcept {
        std::vector <std::pair <Type, Type>> res;
        res.reserve(n);
        res.emplace_back(0, y[0]);
        for (std::size_t i = 1; i < n; ++i) {
            const auto k = (y[i] - y[i - 1]) / (x[i] - x[i - 1]);
            const auto b = y[i] - x[i] * k;
            res.emplace_back(k, b);
        }
        return res;
    }

    [[nodiscard]] inline ComputeFunction auto_select_function(bool is_extra_bound) const noexcept {
        if (n < 16)
            return is_extra_bound
                   ? &PiecewiseLinearCharacteristic::compute_EL
                   : &PiecewiseLinearCharacteristic::compute_SL;
        return is_extra_bound
               ? &PiecewiseLinearCharacteristic::compute_EB
               : &PiecewiseLinearCharacteristic::compute_SB;
    }

    public:
    [[maybe_unused]] PiecewiseLinearCharacteristic(Vec&& x, Vec&& y, bool is_extra_bound = false) noexcept :
            args(std::move(x)), values(std::move(y)), n(args.size()),
            x(args.data()), y(values.data()),
            kb_coeffs(compute_kb()), kb(kb_coeffs.data()),
            compute(auto_select_function(is_extra_bound)) {
    }

    [[maybe_unused]] PiecewiseLinearCharacteristic(const Vec& x, const Vec& y, bool is_extra_bound = false) noexcept :
            args(x), values(y), n(args.size()),
            x(args.data()), y(values.data()),
            kb_coeffs(compute_kb()), kb(kb_coeffs.data()),
            compute(auto_select_function(is_extra_bound)) {
    }

    [[nodiscard]] inline Type operator()(const Type& in) const noexcept {
        return (this->*compute)(in);
    }

    [[nodiscard]] inline Type compute_SL(const Type& in) const noexcept {
        if (in <= x[0])
            return y[0];

        if (in >= x[n - 1])
            return y[n - 1];

        for (std::size_t i = 1; i < n; ++i)
            if (in <= x[i]) {
                const auto& [k, b] = kb[i];
                return k * in + b;
            }

        return 0;
    }

    [[nodiscard]] inline Type compute_EL(const Type& in) const noexcept {
        if (in <= x[0]) {
            const auto& [k, b] = kb[1];
            return k * in + b;
        }

        if (in >= x[n - 1]) {
            const auto& [k, b] = kb[n - 1];
            return k * in + b;
        }

        for (std::size_t i = 1; i < n; ++i)
            if (in < x[i]) {
                const auto& [k, b] = kb[i];
                return k * in + b;
            }

        return 0;
    }

    [[nodiscard]] inline Type compute_SB(const Type& in) const noexcept {
        if (in <= x[0])
            return y[0];

        if (in >= x[n - 1])
            return y[n - 1];

        const std::size_t i = std::upper_bound(x + 1, x + n, in) - x;
        const auto& [k, b] = kb[i];
        return k * in + b;
    }

    [[nodiscard]] inline Type compute_EB(const Type& in) const noexcept {
        if (in <= x[0]) {
            const auto& [k, b] = kb[1];
            return k * in + b;
        }

        if (in >= x[n - 1]) {
            const auto& [k, b] = kb[n - 1];
            return k * in + b;
        }

        const std::size_t i = std::upper_bound(x + 1, x + n, in) - x;
        const auto& [k, b] = kb[i];
        return k * in + b;
    }

    [[nodiscard]] inline std::string getX() const noexcept {
        std::stringstream line;
        line << '{' << x[0];
        for (std::size_t i = 1; i < n; ++i)
            line << ", " << x[i];
        line << '}';
        return line.str();
    }

    [[nodiscard]] inline std::string getY() const noexcept {
        std::stringstream line;
        line << '{' << y[0];
        for (std::size_t i = 1; i < n; ++i)
            line << ", " << y[i];
        line << '}';
        return line.str();
    }
};
}