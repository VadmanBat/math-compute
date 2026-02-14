#pragma once

#include "constants/config.hxx"
#include <vector>

namespace nrcki {
class PortsBase {
    using size = types::size;

public:
    const size input_count;
    const size output_count;

    PortsBase(const size in, const size out) :
        input_count(in),
        output_count(out) {
    }

    virtual ~PortsBase() = default;

    virtual void allocate(void* data) = 0;

    [[nodiscard]] virtual size getTypeHash() const = 0;
    [[nodiscard]] virtual size getTypeSize() const = 0;

    [[nodiscard]] bool isOrigin() const {
        return input_count == 0;
    }

    [[nodiscard]] virtual void* getOutputPort(size index) const = 0;
    virtual void setInputPort(size index, void* port) = 0;
};

template <typename T>
class Ports final : public PortsBase {
    using size = types::size;

public:
    std::vector<T*> inputs;
    T* outputs = nullptr;

    Ports(const size in, const size out) :
        PortsBase(in, out),
        inputs(in) {
    }

    void allocate(void* data) override {
        outputs = static_cast<T*>(data);
    }

    [[nodiscard]] size getTypeHash() const override {
        return types::type_hash<T>();
    }

    [[nodiscard]] size getTypeSize() const override {
        return sizeof(T);
    }

    [[nodiscard]] void* getOutputPort(size index) const override {
        return outputs + index;
    }

    void setInputPort(size index, void* port) override {
        inputs[index] = static_cast<T*>(port);
    }
};
}