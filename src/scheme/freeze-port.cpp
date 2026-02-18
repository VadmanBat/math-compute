#include "nrcki/scheme.h"

namespace nrcki {
void Scheme::freezePort(const size_t block_index, size_t abs_input_port, const double value) {
    const std::pair key{block_index, abs_input_port};
    if (const auto it = frozen_ports.find(key); it != frozen_ports.end()) {
        *it->second.value = value;
        return;
    }

    const auto* block  = blocks[block_index].get();
    void* original_ptr = block->getInputPortAbsolute(abs_input_port);
    auto* new_value    = new double(value);

    block->setInputPortAbsolute(abs_input_port, new_value);
    frozen_ports[key] = {original_ptr, new_value};
}

void Scheme::unfreezePort(size_t block_index, size_t abs_input_port) {
    const std::pair key{block_index, abs_input_port};

    const auto it = frozen_ports.find(key);
    if (it == frozen_ports.end())
        return;

    blocks[block_index]->setInputPortAbsolute(abs_input_port, it->second.original);

    delete it->second.value;
    frozen_ports.erase(it);
}

void Scheme::unfreezeAllPorts() {
    for (auto& [key, frozen] : frozen_ports) {
        auto [block_index, abs_input_port] = key;
        blocks[block_index]->setInputPortAbsolute(abs_input_port, frozen.original);
        delete frozen.value;
    }
    frozen_ports.clear();
}
}