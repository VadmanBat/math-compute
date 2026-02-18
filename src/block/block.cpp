#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <map>

namespace nrcki {
using types::size;

void Block::register_ports(PortsBase* base) {
    ports_bases[base->getTypeHash()] = base;
}

std::unordered_map <size, size> Block::inputs() const {
    std::unordered_map <size, size> res;
    for (const auto [type_hash, ports] : ports_bases)
        res[type_hash] = ports->input_count;
    return res;
}

std::unordered_map <size, size> Block::outputs() const {
    std::unordered_map <size, size> res;
    for (const auto [type_hash, ports] : ports_bases)
        res[type_hash] = ports->output_count;
    return res;
}

void Block::outputs(std::unordered_map <size, void*> memory) {
    for (const auto [type_hash, ports] : ports_bases)
        ports->allocate(memory[type_hash]);
}

Block::~Block() {
    for (auto [type_hash, ports] : ports_bases)
        delete ports;
}

void Block::initIndices() {
    size total_input_count = 0;
    size total_output_count = 0;
    std::map <size, PortsBase*> sorted_bases;
    for (const auto [type_hash, ports] : ports_bases) {
        total_input_count += ports->input_count;
        total_output_count += ports->output_count;
        sorted_bases[type_hash] = ports;
    }
    absolute_input_ports.reserve(total_input_count);
    absolute_output_ports.reserve(total_output_count);
    for (const auto [type_hash, ports] : sorted_bases) {
        for (size i = 0; i < ports->input_count; ++i)
            absolute_input_ports.emplace_back(type_hash, i);
        for (size i = 0; i < ports->output_count; ++i)
            absolute_output_ports.emplace_back(ports->getOutputPort(i));
    }
}

void Block::setInputPortAbsolute(const size index, void* port) const {
    const auto [type_hash, i] = absolute_input_ports[index];
    ports_bases.at(type_hash)->setInputPort(i, port);
}

void Block::setInputPortRelative(const size type_hash, const size index, void* port) const {
    ports_bases.at(type_hash)->setInputPort(index, port);
}

void* Block::getOutputPortAbsolute(const size index) const {
    return absolute_output_ports[index];
}

void* Block::getOutputPortRelative(const size type_hash, const size index) const {
    return ports_bases.at(type_hash)->getOutputPort(index);
}

void* Block::getInputPortAbsolute(const size index) const {
    const auto [type_hash, i] = absolute_input_ports[index];
    return ports_bases.at(type_hash)->getInputPort(i);
}

const std::vector <void*>& Block::getOutputPortsAbsolute() const {
    return absolute_output_ports;
}
}