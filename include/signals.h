#pragma once

#include "constants/config.hxx"

namespace nrcki {
class SignalsBase {
public:
    virtual ~SignalsBase() = default;

    [[nodiscard]] virtual size_t getNumInputs() const = 0;
    [[nodiscard]] virtual size_t getNumOutputs() const = 0;
    virtual void setInputPointer(void* ptr) = 0;
    virtual void setOutputPointer(void* ptr) = 0;

    [[nodiscard]] virtual size_t getInputOffset() const = 0;
    [[nodiscard]] virtual size_t getOutputOffset() const = 0;

    virtual void setInputOffset(size_t off);
    virtual void setOutputOffset(size_t off);
};

template <typename T = double>
class Signals final : public SignalsBase {
public:
    size_t n_inputs      = 0;
    size_t n_outputs     = 0;
    T* inputs            = nullptr;
    T* outputs           = nullptr;
    size_t input_offset  = 0;
    size_t output_offset = 0;

    explicit Signals(const size_t num_in = 0, const size_t num_out = 0)
        : n_inputs(num_in), n_outputs(num_out) {
    }

    [[nodiscard]] size_t getNumInputs() const override { return n_inputs; }
    [[nodiscard]] size_t getNumOutputs() const override { return n_outputs; }

    void setInputPointer(void* ptr) override { inputs = static_cast<T*>(ptr); }
    void setOutputPointer(void* ptr) override { outputs = static_cast<T*>(ptr); }

    void setInputOffset(const size_t off) override { input_offset = off; }
    void setOutputOffset(const size_t off) override { output_offset = off; }

    [[nodiscard]] size_t getInputOffset() const override { return input_offset; }
    [[nodiscard]] size_t getOutputOffset() const override { return output_offset; }
};
}