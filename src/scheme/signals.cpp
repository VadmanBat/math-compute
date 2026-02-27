#include "nrcki/scheme.h"

namespace nrcki {
void Scheme::build_signals() {
    size_t total_in  = 0;
    size_t total_out = 0;

    for (const auto& b : blocks)
        if (const auto* sig = b->getSignals()) {
            total_in  += sig->getNumInputs();
            total_out += sig->getNumOutputs();
        }

    input_buffer.assign(total_in, 0.0);
    output_buffer.assign(total_out, 0.0);

    double* in_ptr  = input_buffer.data();
    double* out_ptr = output_buffer.data();

    size_t in_offset  = 0;
    size_t out_offset = 0;
    for (const auto& b : blocks)
        if (auto* sig = b->getSignals()) {
            if (sig->getNumInputs() > 0) {
                sig->setInputPointer(in_ptr);
                sig->setInputOffset(in_offset);
                in_ptr += sig->getNumInputs();
                in_offset += sig->getNumInputs();
            }
            if (sig->getNumOutputs() > 0) {
                sig->setOutputPointer(out_ptr);
                sig->setOutputOffset(out_offset);
                out_ptr += sig->getNumOutputs();
                out_offset += sig->getNumOutputs();
            }
        }
}

void Scheme::setInputs(const double* values, const size_t count) {
    std::ranges::copy(values, values + count, input_buffer.data());
}

void Scheme::getOutputs(double* values) const {
    std::ranges::copy(output_buffer, values);
}

size_t Scheme::getOutputCount() const {
    return output_buffer.size();
}
}