#pragma once

#include "constants/config.hxx"

#include <memory>
#include <vector>

namespace nrcki {
struct PortInfo {
    bool is_constant        = false;
    types::size block_index = 0, relative_index = 0;
};

class Block;

class Context {
    using Type      = types::real;
    using size      = types::size;
    using string    = types::string;
    using PortsInfo = std::unordered_map<void*, PortInfo>;
    using BlocksVec = std::vector<std::unique_ptr<Block>>;

public:
    static constexpr Type dt_sec     = 1e-1;         // Шаг интегрирования [сек]
    static constexpr types::time sec = 1e6;          // Основание времени [микро-секунды (мкс)]
    static constexpr types::time dt  = sec * dt_sec; // Шаг интегрирования [мкс]

    const types::time& time; // Абсолютное время расчёта схемы [мкс]

    PortsInfo& ports_info;
    const BlocksVec& blocks;

    explicit Context(const types::time& time, PortsInfo& ports_info, const BlocksVec& blocks) :
        time(time),
        ports_info(ports_info),
        blocks(blocks) {
    }

    virtual ~Context() = default;

    [[nodiscard]] virtual constexpr Type parameter(const string& name) const = 0;
};
}