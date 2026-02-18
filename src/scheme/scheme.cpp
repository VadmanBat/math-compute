//
// Created by Vadim on 06.05.2025.
//

#include "nrcki/scheme.h"
#include "ports.hpp"

#include <algo/kahn.hpp>

#include <algorithm>
#include <set>

namespace nrcki {
void Scheme::compute_calculation_order() {
    std::vector<size> prioritized_breakers;
    prioritized_breakers.reserve(blocks.size());
    for (size i = 0; i < blocks.size(); ++i)
        if (blocks[i]->canUntieLoop())
            prioritized_breakers.push_back(i);

    time = 0;
    sorted_blocks.clear();
    active_sorted_blocks.clear();
    compute_sorted_blocks.clear();

    const algo::KahnTopologicalSort sort(direct_graph, prioritized_breakers);
    for (const auto index : sort.result())
        sorted_blocks.push_back(blocks[index].get());

    std::vector<Block*> explicit_blocks;
    for (const auto block : sorted_blocks) {
        block->init();
        if (!block->tryMakeConstant()) {
            active_sorted_blocks.push_back(block);
            block->isImplicitCompute() ? compute_sorted_blocks.push_back(block) : explicit_blocks.push_back(block);
        }
    }

    std::ranges::reverse(compute_sorted_blocks); // обратный порядок блоков для неявного вычисления
    compute_sorted_blocks.insert(compute_sorted_blocks.end(), explicit_blocks.begin(), explicit_blocks.end());
}

void Scheme::init_indices() {
    for (const auto& block : blocks)
        block->initIndices();

    relative_input_index.clear();
    absolute_input_index.clear();
    absolute_output_index.clear();
    relative_output_index.clear();

    std::set<size> sorted_type_hashes;
    for (auto& [type_hash, bytes] : port_memory) {
        const size count             = total_outputs[type_hash];
        const size type_size         = types::type_size(type_hash);
        auto& relative_output_points = relative_output_index[type_hash];
        byte* data                   = bytes.data();
        for (size i = 0; i < count; ++i) {
            relative_output_points.emplace_back(data + i * type_size);
            ports_info[data + i * type_size].relative_index = relative_output_points.size() - 1;
        }
        sorted_type_hashes.insert(type_hash);
    }

    for (size i = 0; i < blocks_count; ++i) {
        int abs_input_index = -1;
        auto inputs         = blocks[i]->inputs();
        auto outputs        = blocks[i]->outputs();
        for (const auto& type_hash : sorted_type_hashes) {
            const auto input_count      = inputs[type_hash];
            auto& relative_input_points = relative_input_index[type_hash];
            for (size j = 0; j < input_count; ++j) {
                absolute_input_index.emplace_back(i, ++abs_input_index);
                relative_input_points.emplace_back(i, j);
            }
            for (auto port_point : blocks[i]->getOutputPortsAbsolute()) {
                absolute_output_index.emplace_back(type_hash, port_point);
                ports_info[port_point].block_index = i;
            }
        }
    }
}

void Scheme::allocate_memory() {
    total_outputs.clear();
    for (const auto& block : blocks)
        for (const auto [type_hash, count] : block->outputs())
            total_outputs[type_hash] += count;

    for (const auto& [type_hash, count] : total_outputs)
        port_memory[type_hash].resize(count * types::type_size(type_hash));

    std::unordered_map<size, size> offsets;
    for (const auto& block : blocks) {
        std::unordered_map<size, void*> block_memory;
        for (const auto& [type_hash, count] : block->outputs()) {
            block_memory[type_hash] = port_memory[type_hash].data() + offsets[type_hash];
            offsets[type_hash]      += count * types::type_size(type_hash);
        }

        block->outputs(block_memory);
    }

    build_signals();
    init_indices();
}

void Scheme::compute(const uint64_t steps) {
    for (size i = 0; i < steps; ++i) {
        time += dt;
        for (const auto block : compute_sorted_blocks)
            block->compute();
    }
    dt_count += steps;
}

void Scheme::computeSync(const uint64_t steps) {
    Time += steps * sync_step;
    while (time < Time) {
        time += dt;
        for (const auto block : compute_sorted_blocks)
            block->compute();
        ++dt_count;
    }
}

std::vector<uint8_t> Scheme::getDoubles() {
    return port_memory[types::type_hash<double>()];
}
}