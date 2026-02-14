#include "nrcki/scheme.h"

namespace nrcki {
#define SIZE 2
/// [0: output_port, 1: input_port]
[[maybe_unused]] void Scheme::setAbsoluteLinks(int n, const link* links) {
    allocate_memory();
    const auto last = links + SIZE * n;
    for (auto link = links; link != last; link += SIZE) {
        const auto [block_index, port_index] = absolute_input_index[link[1]];
        const auto port_point                = absolute_output_index[link[0]].second;
        blocks[block_index]->setInputPortAbsolute(port_index, port_point);
        //reverse_graph[block_index].push_back(ports_info[port_point].block_index);
        direct_graph[ports_info[port_point].block_index].push_back(block_index);
    }

    compute_calculation_order();
}
#undef SIZE

#define SIZE 3
/// [0: type_hash, 1: output_port, 2: input_port]
[[maybe_unused]] void Scheme::setRelativeLinks(int n, const link* links) {
    allocate_memory();

    const auto last = links + SIZE * n;
    for (auto link = links; link != last; link += SIZE) {
        uint64_t type_hash                   = link[0];
        const auto [block_index, port_index] = relative_input_index[type_hash][link[2]];
        const auto port_point                = relative_output_index[type_hash][link[1]];
        blocks[block_index]->setInputPortRelative(type_hash, port_index, port_point);
        //reverse_graph[block_index].push_back(ports_info[port_point].block_index);
        direct_graph[ports_info[port_point].block_index].push_back(block_index);
    }

    compute_calculation_order();
}
#undef SIZE

#define SIZE 4
/// [0: output_block, 1: output_port, 2: input_block, 3: input_port]
[[maybe_unused]] void Scheme::setAbsoluteBlockLinks(int n, const link* links) {
    allocate_memory();

    const auto last = links + SIZE * n;
    for (auto link = links; link != last; link += SIZE) {
        blocks[link[2]]->setInputPortAbsolute(link[3], blocks[link[0]]->getOutputPortAbsolute(link[1]));
        //reverse_graph[link[2]].push_back(link[0]);
        direct_graph[link[0]].push_back(link[2]);
    }

    compute_calculation_order();
}
#undef SIZE

#define SIZE 5
/// [0: type_hash, 1: output_block, 2: output_port, 3: input_block, 4: input_port]
[[maybe_unused]] void Scheme::setRelativeBlockLinks(int n, const link* links) {
    allocate_memory();

    const auto last = links + SIZE * n;
    for (auto link = links; link != last; link += SIZE) {
        const uint64_t type_hash = link[0];
        blocks[link[3]]->setInputPortRelative(type_hash, link[4],
                                              blocks[link[1]]->getOutputPortRelative(type_hash, link[2]));
        //reverse_graph[link[3]].push_back(link[1]);
        direct_graph[link[1]].push_back(link[3]);
    }

    compute_calculation_order();
}
#undef SIZE
}