#pragma once

#include "constants/config.hxx"
#include "context.hpp"
#include "signals.h"

#include <vector>
#include <unordered_map>


namespace nrcki {
class PortsBase;

class Block {
    using size = types::size;

    // Относительная индексация:
    std::unordered_map<size, PortsBase*> ports_bases;
    // Абсолютная индексация:
    std::vector<std::pair<size, size>> absolute_input_ports;
    std::vector<void*> absolute_output_ports;

    inline static size count = -1;

    int flags = 0;

protected:
    const Context& context;
    const size id;

    enum FlagType {
        CONSTANT = 1 << 0,
        CAN_UNTIE_LOOP = 1 << 1,
        IMPLICIT_COMPUTE = 1 << 2,
    };

    SignalsBase* signals = nullptr;

    void register_ports(PortsBase* base);
    void register_signals(SignalsBase* s) { signals = s; }

public:
    explicit Block(const Context& context) : context(context), id(++count) {
    }

    virtual ~Block();

    std::unordered_map<size, size> inputs() const;
    std::unordered_map<size, size> outputs() const;
    void outputs(std::unordered_map<size, void*> memory);
    SignalsBase* getSignals() const { return signals; }

    void initIndices();

    // Виртуальный расчёт:
    virtual void init() const { this->compute(); }

    virtual void compute() const {
    }

    // Поддержка флагов:
    void toggleFlag(const int flag) { flags ^= flag; };

    bool isConstant() const { return flags & CONSTANT; }
    bool canUntieLoop() const { return flags & CAN_UNTIE_LOOP; }
    bool isImplicitCompute() const { return flags & IMPLICIT_COMPUTE; }

    virtual bool tryMakeConstant() { return false; }

    // Кодогенерация:
    [[maybe_unused]] virtual types::string printMemory() const { return ""; }
    [[maybe_unused]] virtual types::string printInit() const { return this->printSource(); }
    [[maybe_unused]] virtual types::string printSource() const { return ""; }

#define CODE_NAME_IN(PORTS, INDEX) \
types::type_codename(PORTS->getTypeHash()) + \
'[' + std::to_string(context.ports_info.at(PORTS->inputs[INDEX]).relative_index) + ']'
#define CODE_NAME_OUT(PORTS, INDEX) \
types::type_codename(PORTS->getTypeHash()) + \
'[' + std::to_string(context.ports_info.at(&PORTS->outputs[INDEX]).relative_index) + ']'
#define CODE_NAME_SIGNAL_IN(SIGNALS, INDEX) (\
"input_signals[" + std::to_string((SIGNALS)->getInputOffset() + static_cast<size_t>(INDEX)) + ']')
#define CODE_NAME_SIGNAL_OUT(SIGNALS, INDEX) (\
"output_signals[" + std::to_string((SIGNALS)->getOutputOffset() + static_cast<size_t>(INDEX)) + ']')
#define CODE_NAME_TYPE(TYPE) \
(types::type_name(types::type_hash<TYPE>()))
#define CODE_NAME_INF(TYPE) \
("std::numeric_limits<" + types::type_name(types::type_hash<TYPE>()) + "::infinity()")
#define CODE_NAME_MIN(TYPE) \
("std::numeric_limits<" + types::type_name(types::type_hash<TYPE>()) + "::min()")
#define CODE_NAME_MAX(TYPE) \
("std::numeric_limits<" + types::type_name(types::type_hash<TYPE>()) + "::max()")
#define REGISTER_VAR(NAME) \
const auto var_ ## NAME = #NAME"_" + std::to_string(Block::id);
#define CODE_NAME_SEC "sec"

    // Универсальные методы работы с портами:
    [[maybe_unused]] virtual void setInputPortAbsolute(size index, void* port) const;
    [[maybe_unused]] virtual void setInputPortRelative(size type_hash, size index, void* port) const;

    [[maybe_unused]] virtual void* getInputPortAbsolute(size index) const;
    [[maybe_unused]] virtual void* getOutputPortAbsolute(size index) const;
    [[maybe_unused]] virtual void* getOutputPortRelative(size type_hash, size index) const;

    [[maybe_unused]] const std::vector<void*>& getOutputPortsAbsolute() const;
};
}