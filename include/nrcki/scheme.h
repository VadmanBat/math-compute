#pragma once

#include "constants/config.hxx"
#include "context.hpp"

#include "block.h"

#include <vector>
#include <memory>
#include <unordered_map>
#include <map>
#include <iostream>

namespace nrcki {
/// Класс расчётной схемы
class Scheme final : public Context {
    using Type   = types::real;   // Вещественный тип
    using byte   = types::byte;   // Тип для байта (8 бит)
    using size   = types::size;   // Тип для размеров, индексации
    using link   = types::link;   // Тип для индексации связей
    using string = types::string; // Тип для строк

    size dt_count     = 0;                      // Число шагов интегрирования
    types::time time  = 0, Time = 0, sync_step; // Абсолютное время
    size blocks_count = 0;                      // Количество блоков

    /// Выделение памяти для портов:
    std::unordered_map<size, std::vector<byte>> port_memory; /// Байты выходных портов
    std::unordered_map<size, size> total_outputs;            /// Количество выходных портов: type_hash -> count

    /// Выделение памяти для сигналов:
    std::vector<double> input_buffer;  // весь внешний вход схемы (непрерывно)
    std::vector<double> output_buffer; // весь внешний выход схемы

    /// Абсолютная и относительная индексация:
    std::vector<std::pair<size, void*>> absolute_output_index;          // index -> [type_hash, ptr]
    std::unordered_map<size, std::vector<void*>> relative_output_index; // type_hash -> index -> ptr

    std::vector<std::pair<size, size>> absolute_input_index; // index -> [block, abs_index]
    std::unordered_map<size, std::vector<std::pair<size, size>>> relative_input_index;
    // type_hash -> index -> [block, abs_index]

    std::unordered_map<void*, PortInfo> ports_info;

    /// Блоки:
    std::vector<std::unique_ptr<Block>> blocks;
    std::unordered_map<string, Type> parameters;

    std::unordered_map<size, std::vector<size>> direct_graph;
    std::vector<Block*> sorted_blocks, active_sorted_blocks, compute_sorted_blocks;

    struct FrozenPort {
        void* original = nullptr;
        double* value  = nullptr;
    };

    std::map<std::pair<size, size>, FrozenPort> frozen_ports;

    void build_signals();

    void init_indices();
    void allocate_memory();
    void compute_calculation_order();

    // Абсолютная индексация
    template <typename T>
    T* get_absolute_output_port(size index) const {
        if (index >= absolute_output_index.size())
            return nullptr;
        const auto [hash, ptr] = absolute_output_index[index];
        return static_cast<T*>(ptr);
    }

    // Относительная индексация
    template <typename T>
    T* get_relative_output_port(size index) const {
        const auto hash = types::type_hash<T>();
        if (auto it = relative_output_index.find(hash); it != relative_output_index.end())
            if (index < total_outputs.at(hash))
                return static_cast<T*>(it->second[index]);
        return nullptr;
    }

public:
    Scheme() : Context(time, ports_info, blocks) {
    }

    ~Scheme() override = default;

    void setSteps(double sync, double delta_time) {
        sync_step = sync * Context::sec;
    }

    void assign(uint32_t count, const uint8_t* data);

    constexpr Type parameter(const string& name) const override {
        return parameters.at(name);
    }

    // Добавление блоков:
    // Источники (Sources):
    void addConstant(double c = 1);
    void addStep(double time = 10, double yk = 1, double y0 = 0);
    void addLinearSource(double k = 1, double b = 0);
    void addSinusSource(double a = 1, double w = 1, double f = 0);

    // Арифметические операторы (Operators):
    void addSummator(int n, double* coefficients);
    void addMultiplier(int n = 2);
    void addDivider(double value_if_div_null);
    void addAbsoluteValue();
    void addSign();

    // Логические операторы (Logical):
    void addAnd(int n = 2);
    void addOr(int n = 2);
    void addNot();
    void addEqual();
    void addNotEqual();
    void addLess();
    void addLessOrEqual();
    void addGreater();
    void addGreaterOrEqual();

    // Динамические (Dynamic):
    void addIntegrator(double k = 1, double y0 = 0);
    void addInertial(double k = 1, double T = 1, double y0 = 0);
    void addInertialDifferential(double k = 1, double T = 1, double y0 = 0);
    void addOscillatory(double k = 1, double T = 1, double b = 0.5, double y0 = 0, double dy0 = 0);
    void addStepDelay(double y0 = 0);

    // Задержки (Delays):
    void addDelayOn(double T = 1);
    void addDelayOff(double T = 1);
    void addDelayOnOff(double T_on = 1, double T_off = 1);

    // Интерполяция (Interpolation):
    void addPiecewiseLinear(int n, double* x, double* y, bool is_extra_bound = false);

    // Нелинейные (Nonlinear):
    void addSaturation(double x1 = 0, double x2 = 100, double y1 = 0, double y2 = 1);
    void addDeadband(double x1 = -1, double x2 = 1, double k = 1);
    void addSaturationDeadband(double x1    = -10, double x2   = 10,
                               double y1    = -1, double y2    = 1,
                               double db_x1 = -1, double db_x2 = 1
        );
    void addHysteresis(double x1 = -1, double x2 = 1, double y1 = -1, double y2 = 1, bool y0 = false);
    void addHysteresisDeadband(
        double x1    = -10, double x2   = 10,
        double y1    = -1, double y2    = 1,
        double db_x1 = -1, double db_x2 = 1,
        int y0       = 0
        );
    void addLowThreshold(double activation = 95, double deactivation = 100);
    void addHighThreshold(double activation = 105, double deactivation = 100);
    void addVariableHysteresis();
    void addVariableHysteresisPlus();
    void addVariableHysteresisMinus();

    // Импульсы (Pulses):
    void addPulse(double T = 1);
    void addRisingPulse();
    void addFallingPulse();
    void addLongPulse(double T = 1);
    void addShortPulse(double T = 1);
    void addDebounce(double T = 1, char type = 't');

    // Ключи (Switches):
    void addToggleSwitch();

    // Триггеры (Triggers):
    void addRsTrigger(bool y0 = false);
    void addSrTrigger(bool y0 = false);
    void addStrTrigger(char type = 'r', bool y0 = false);
    void addRtsTrigger(char type = 'r', bool y0 = false);

    // Задание связей:
    [[maybe_unused]] void setAbsoluteLinks(int n, const link* links);
    [[maybe_unused]] void setRelativeLinks(int n, const link* links);
    [[maybe_unused]] void setAbsoluteBlockLinks(int n, const link* links);
    [[maybe_unused]] void setRelativeBlockLinks(int n, const link* links);

    void compute(uint64_t steps = 1);
    void computeSync(uint64_t steps = 1);

    const std::unordered_map<size, size>& getPortsCount() const { return total_outputs; }
    const std::vector<Block*>& getSortedBlocks() const { return sorted_blocks; }
    const std::vector<Block*>& getActiveBlocks() const { return active_sorted_blocks; }
    const std::vector<Block*>& getComputeBlocks() const { return compute_sorted_blocks; }

    template <typename T>
    void printOutputs() {
        for (int i = 0; i < total_outputs[types::type_hash<T>()]; ++i)
            std::cout << *get_relative_output_port<T>(i) << ' ';
        std::cout << '\n';
    }

    template <typename T>
    T getAbsoluteOutputPort(size index) const {
        const auto [hash, ptr] = absolute_output_index[index];
        return *static_cast<T*>(ptr);
    }

    template <typename T>
    void printOutputPorts() const {
        const auto n = total_outputs.at(types::type_hash<T>());
        for (std::size_t i = 0; i < n; ++i)
            std::cout << getAbsoluteOutputPort<double>(i) << ' ';
        std::cout << '\n';
    }

    std::vector<uint8_t> getDoubles();

    void setInputs(const double* values, size_t count);
    void getOutputs(double* values) const;

    void freezePort(size_t block_index, size_t abs_input_port, double value);
    void unfreezePort(size_t block_index, size_t abs_input_port);
    void unfreezeAllPorts();
};
}