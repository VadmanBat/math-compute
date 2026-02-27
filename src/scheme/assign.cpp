#include "nrcki/scheme.h"
#include "constants/block-list.hxx"

#include "blocks/delays/delay-on.hpp"
#include "blocks/delays/delay-off.hpp"
#include "blocks/delays/delay-on-off.hpp"
#include "blocks/delays/delay-on-dynamic.hpp"
#include "blocks/delays/delay-off-dynamic.hpp"
#include "blocks/delays/delay-on-off-dynamic.hpp"

#include "blocks/dynamic/integrator.hpp"
#include "blocks/dynamic/inertial.hpp"
#include "blocks/dynamic/inertial-differential.hpp"
#include "blocks/dynamic/oscillatory.hpp"
#include "blocks/dynamic/step-delay.hpp"

#include "blocks/interpolation/piecewise-linear.hpp"

#include "blocks/logical/and.hpp"
#include "blocks/logical/or.hpp"
#include "blocks/logical/xor.hpp"
#include "blocks/logical/not.hpp"
#include "blocks/logical/equal.hpp"
#include "blocks/logical/not-equal.hpp"
#include "blocks/logical/less.hpp"
#include "blocks/logical/less-or-equal.hpp"
#include "blocks/logical/greater.hpp"
#include "blocks/logical/greater-or-equal.hpp"
#include "blocks/logical/and-not.hpp"
#include "blocks/logical/or-not.hpp"
#include "blocks/logical/xor-not.hpp"

#include "blocks/nonlinear/saturation.hpp"
#include "blocks/nonlinear/deadband.hpp"
#include "blocks/nonlinear/saturation-deadband.hpp"
#include "blocks/nonlinear/hysteresis.hpp"
#include "blocks/nonlinear/hysteresis-deadband.hpp"
#include "blocks/nonlinear/hysteresis-dynamic.hpp"

#include "blocks/nonlinear/low-threshold.hpp"
#include "blocks/nonlinear/high-threshold.hpp"

#include "blocks/nonlinear/variable-hysteresis.hpp"
#include "blocks/nonlinear/variable-hysteresis-plus.hpp"
#include "blocks/nonlinear/variable-hysteresis-minus.hpp"

#include "blocks/operators/summator.hpp"
#include "blocks/operators/multiplier.hpp"
#include "blocks/operators/divider.hpp"
#include "blocks/operators/absolute-value.hpp"
#include "blocks/operators/sign.hpp"

#include "blocks/pulses/rising-pulse.hpp"
#include "blocks/pulses/falling-pulse.hpp"
#include "blocks/pulses/pulse.hpp"
#include "blocks/pulses/short-pulse.hpp"
#include "blocks/pulses/long-pulse.hpp"
#include "blocks/pulses/pulse-dynamic.hpp"
#include "blocks/pulses/short-pulse-dynamic.hpp"
#include "blocks/pulses/long-pulse-dynamic.hpp"

#include "blocks/pulses/debounce-on.hpp"
#include "blocks/pulses/debounce-off.hpp"
#include "blocks/pulses/debounce-on-off.hpp"

#include "blocks/signals/ext-in-signal.hpp"
#include "blocks/signals/ext-out-signal.hpp"
#include "blocks/signals/int-in-signal.hpp"
#include "blocks/signals/int-out-signal.hpp"
#include "blocks/signals/plot.hpp"

#include "blocks/sources/constant.hpp"
#include "blocks/sources/step.hpp"
#include "blocks/sources/linear-source.hpp"
#include "blocks/sources/sinus-source.hpp"

#include "blocks/switches/toggle-switch.hpp"

#include "blocks/triggers/rs-trigger.hpp"
#include "blocks/triggers/sr-trigger.hpp"

#include "blocks/triggers/t-triggers/t-trigger-r.hpp"
#include "blocks/triggers/t-triggers/t-trigger-f.hpp"
#include "blocks/triggers/t-triggers/t-trigger-b.hpp"
#include "blocks/triggers/t-triggers/t-trigger-l.hpp"

#include "blocks/triggers/t-triggers/rts-r-trigger.hpp"
#include "blocks/triggers/t-triggers/rts-f-trigger.hpp"
#include "blocks/triggers/t-triggers/rts-b-trigger.hpp"
#include "blocks/triggers/t-triggers/rts-l-trigger.hpp"

#include "blocks/triggers/t-triggers/str-r-trigger.hpp"
#include "blocks/triggers/t-triggers/str-f-trigger.hpp"
#include "blocks/triggers/t-triggers/str-b-trigger.hpp"
#include "blocks/triggers/t-triggers/str-l-trigger.hpp"

namespace nrcki {
void Scheme::assign(uint32_t count, const uint8_t* data) {
    double a, b, f, w, k, T, y0, dy0;
    double T_on, T_off;

    uint16_t n;
    std::vector<double> x, y, coeffs;
    bool is_extra_bound;

    double x1, x2, y1, y2;
    double db_x1, db_x2;
    bool bin_state_y0;
    int8_t tern_state_y0;

    double value_if_div_null;

    char type;

#define read(VAR) __builtin_memcpy(&VAR, data, sizeof(VAR)); data += sizeof(VAR);
#define read_arr(VAR) VAR.resize(n); for (auto& item : VAR) { read(item) }
    uint8_t block_id;
    for (size i = 0; i < count; ++i) {
        read(block_id)
        switch (static_cast<BlockID>(block_id)) {
            case BlockID::DelayOn: read(type)
                if (type == 1)
                    blocks.push_back(std::make_unique<DelayOnDynamic>(*this));
                else {
                    read(T)
                    blocks.push_back(std::make_unique<DelayOn>(*this, T));
                }
                break;
            case BlockID::DelayOff: read(type)
                if (type == 1)
                    blocks.push_back(std::make_unique<DelayOffDynamic>(*this));
                else {
                    read(T)
                    blocks.push_back(std::make_unique<DelayOff>(*this, T));
                }
                break;
            case BlockID::DelayOnOff: read(type)
                if (type == 1)
                    blocks.push_back(std::make_unique<DelayOnOffDynamic>(*this));
                else {
                    read(T_on)
                    read(T_off)
                    blocks.push_back(std::make_unique<DelayOnOff>(*this, T_on, T_off));
                }
                break;

            case BlockID::Integrator:
                read(k)
                read(y0)
                blocks.push_back(std::make_unique<Integrator>(*this, k, y0));
                break;
            case BlockID::Inertial:
                read(k)
                read(T)
                read(y0)
                blocks.push_back(std::make_unique<Inertial>(*this, k, T, y0));
                break;
            case BlockID::InertialDifferential:
                read(k)
                read(T)
                read(y0)
                blocks.push_back(std::make_unique<InertialDifferential>(*this, k, T, y0));
                break;
            case BlockID::Oscillatory:
                read(k)
                read(T)
                read(b)
                read(y0)
                read(dy0)
                blocks.push_back(std::make_unique<Oscillatory>(*this, k, T, b, y0, dy0));
                break;
            case BlockID::StepDelay: read(y0)
                blocks.push_back(std::make_unique<StepDelay>(*this, y0));
                break;

            case BlockID::PiecewiseLinear:
                read(n)
                read_arr(x)
                read_arr(y)
                read(is_extra_bound)
                blocks.push_back(std::make_unique<PiecewiseLinear>(*this, x, y, is_extra_bound));
                break;

            case BlockID::Or:
                read(n)
                read(bin_state_y0)
                if (bin_state_y0)
                    blocks.push_back(std::make_unique<OrNot>(*this, n));
                else
                    blocks.push_back(std::make_unique<Or>(*this, n));
                break;
            case BlockID::And:
                read(n)
                read(bin_state_y0)
                if (bin_state_y0)
                    blocks.push_back(std::make_unique<AndNot>(*this, n));
                else
                    blocks.push_back(std::make_unique<And>(*this, n));
                break;
            case BlockID::Xor:
                read(n)
                read(bin_state_y0)
                if (bin_state_y0)
                    blocks.push_back(std::make_unique<XorNot>(*this, n));
                else
                    blocks.push_back(std::make_unique<Xor>(*this, n));
                break;
            case BlockID::Not:
                blocks.push_back(std::make_unique<Not>(*this));
                break;
            case BlockID::Equal:
                blocks.push_back(std::make_unique<Equal>(*this));
                break;
            case BlockID::NotEqual:
                blocks.push_back(std::make_unique<NotEqual>(*this));
                break;
            case BlockID::Less:
                blocks.push_back(std::make_unique<Less>(*this));
                break;
            case BlockID::Greater:
                blocks.push_back(std::make_unique<Greater>(*this));
                break;
            case BlockID::LessOrEqual:
                blocks.push_back(std::make_unique<LessOrEqual>(*this));
                break;
            case BlockID::GreaterOrEqual:
                blocks.push_back(std::make_unique<GreaterOrEqual>(*this));
                break;

            case BlockID::Saturation:
                read(x1)
                read(x2)
                read(y1)
                read(y2)
                blocks.push_back(std::make_unique<Saturation>(*this, x1, x2, y1, y2));
                break;
            case BlockID::Deadband:
                read(x1)
                read(x2)
                read(k)
                blocks.push_back(std::make_unique<Deadband>(*this, x1, x2, k));
                break;
            case BlockID::SaturationDeadband:
                read(x1)
                read(x2)
                read(y1)
                read(y2)
                read(db_x1)
                read(db_x2)
                blocks.push_back(std::make_unique<SaturationDeadband>(*this, x1, x2, y1, y2, db_x1, db_x2));
                break;
            case BlockID::Hysteresis: read(type)
                if (type == 1) {
                    read(y1)
                    read(y2)
                    read(bin_state_y0)
                    blocks.push_back(std::make_unique<HysteresisDynamic>(*this, y1, y2, bin_state_y0));
                }
                else {
                    read(x1)
                    read(x2)
                    read(y1)
                    read(y2)
                    read(bin_state_y0)
                    blocks.push_back(std::make_unique<Hysteresis>(*this, x1, x2, y1, y2, bin_state_y0));
                }
                break;
            case BlockID::HysteresisDeadband: read(x1)
                read(x2)
                read(y1)
                read(y2)
                read(db_x1)
                read(db_x2)
                read(tern_state_y0)
                blocks.push_back(
                    std::make_unique<HysteresisDeadband>(*this, x1, x2, y1, y2, db_x1, db_x2, tern_state_y0));
                break;
            case BlockID::LowThreshold:
                read(x1)
                read(x2)
                blocks.push_back(std::make_unique<LowThreshold>(*this, x1, x2));
                break;
            case BlockID::HighThreshold:
                read(x1)
                read(x2)
                blocks.push_back(std::make_unique<HighThreshold>(*this, x1, x2));
                break;
            case BlockID::VariableHysteresis:
                blocks.push_back(std::make_unique<VariableHysteresis>(*this));
                break;
            case BlockID::VariableHysteresisPlus:
                blocks.push_back(std::make_unique<VariableHysteresisPlus>(*this));
                break;
            case BlockID::VariableHysteresisMinus:
                blocks.push_back(std::make_unique<VariableHysteresisMinus>(*this));
                break;

            case BlockID::Summator:
                read(n)
                read_arr(coeffs)
                blocks.push_back(std::make_unique<Summator>(*this, coeffs));
                break;
            case BlockID::Multiplier: read(n)
                blocks.push_back(std::make_unique<Multiplier>(*this, n));
                break;
            case BlockID::Divider: read(value_if_div_null)
                blocks.push_back(std::make_unique<Divider>(*this, value_if_div_null));
                break;
            case BlockID::AbsoluteValue:
                blocks.push_back(std::make_unique<AbsoluteValue>(*this));
                break;
            case BlockID::Sign:
                blocks.push_back(std::make_unique<Sign>(*this));
                break;

            case BlockID::RisingPulse:
                blocks.push_back(std::make_unique<RisingPulse>(*this));
                break;
            case BlockID::FallingPulse:
                blocks.push_back(std::make_unique<FallingPulse>(*this));
                break;
            case BlockID::Pulse: read(type)
                if (type == 1) {
                    blocks.push_back(std::make_unique<PulseDynamic>(*this));
                }
                else {
                    read(T)
                    blocks.push_back(std::make_unique<Pulse>(*this, T));
                }
                break;
            case BlockID::ShortPulse: read(type)
                if (type == 1) {
                    blocks.push_back(std::make_unique<ShortPulseDynamic>(*this));
                }
                else {
                    read(T)
                    blocks.push_back(std::make_unique<ShortPulse>(*this, T));
                }
                break;
            case BlockID::LongPulse: read(type)
                if (type == 1) {
                    blocks.push_back(std::make_unique<LongPulseDynamic>(*this));
                }
                else {
                    read(T)
                    blocks.push_back(std::make_unique<LongPulse>(*this, T));
                }
                break;
            case BlockID::DebounceOn: read(T)
                blocks.push_back(std::make_unique<DebounceOn>(*this, T));
                break;
            case BlockID::DebounceOff: read(T)
                blocks.push_back(std::make_unique<DebounceOff>(*this, T));
                break;
            case BlockID::DebounceOnOff: read(T)
                blocks.push_back(std::make_unique<DebounceOnOff>(*this, T));
                break;

            case BlockID::ExtInSignal: read(y0)
                blocks.push_back(std::make_unique<ExtInSignal>(*this, y0));
                break;
            case BlockID::ExtOutSignal:
                blocks.push_back(std::make_unique<ExtOutSignal>(*this));
                break;
            case BlockID::IntInSignal: read(y0)
                read(n)
                blocks.push_back(std::make_unique<IntInSignal>(*this, y0, n));
                break;
            case BlockID::IntOutSignal:
                blocks.push_back(std::make_unique<IntOutSignal>(*this));
                break;
            case BlockID::Plot: read(tern_state_y0)
                blocks.push_back(std::make_unique<Plot>(*this, tern_state_y0));
                break;

            case BlockID::Constant: read(y0)
                blocks.push_back(std::make_unique<Constant>(*this, y0));
                break;
            case BlockID::Step: read(T)
                read(b)
                read(y0)
                blocks.push_back(std::make_unique<Step>(*this, T, b, y0));
                break;
            case BlockID::LinearSource: read(k)
                read(b)
                blocks.push_back(std::make_unique<LinearSource>(*this, k, b));
                break;
            case BlockID::SinusSource: read(a)
                read(w)
                read(f)
                blocks.push_back(std::make_unique<SinusSource>(*this, a, w, f));
                break;

            case BlockID::ToggleSwitch:
                blocks.push_back(std::make_unique<ToggleSwitch>(*this));
                break;

            case BlockID::RsTrigger: read(bin_state_y0)
                blocks.push_back(std::make_unique<RsTrigger>(*this, bin_state_y0));
                break;
            case BlockID::SrTrigger: read(bin_state_y0)
                blocks.push_back(std::make_unique<SrTrigger>(*this, bin_state_y0));
                break;

            case BlockID::TTrigger: read(type)
                read(bin_state_y0)
                switch (type) {
                    case 'r':
                    case 0:
                        blocks.push_back(std::make_unique<TTriggerR>(*this, bin_state_y0));
                        break;
                    case 'f':
                    case 1:
                        blocks.push_back(std::make_unique<TTriggerF>(*this, bin_state_y0));
                        break;
                    case 'b':
                    case 2:
                        blocks.push_back(std::make_unique<TTriggerB>(*this, bin_state_y0));
                        break;
                    case 'l':
                    case 3:
                        blocks.push_back(std::make_unique<TTriggerL>(*this, bin_state_y0));
                        break;
                    default:
                        std::cerr << "Scheme::assign: неверный тип T-триггера\n"
                            << "i: " << i << ", type: " << type << '\n';
                }
                break;

            case BlockID::RtsTrigger: read(type)
                read(bin_state_y0)
                switch (type) {
                    case 'r':
                    case 0:
                        blocks.push_back(std::make_unique<RtsRTrigger>(*this, bin_state_y0));
                        break;
                    case 'f':
                    case 1:
                        blocks.push_back(std::make_unique<RtsFTrigger>(*this, bin_state_y0));
                        break;
                    case 'b':
                    case 2:
                        blocks.push_back(std::make_unique<RtsBTrigger>(*this, bin_state_y0));
                        break;
                    case 'l':
                    case 3:
                        blocks.push_back(std::make_unique<RtsLTrigger>(*this, bin_state_y0));
                        break;
                    default:
                        std::cerr << "Scheme::assign: неверный тип T-триггера\n"
                            << "i: " << i << ", type: " << type << '\n';
                }
                break;
            case BlockID::StrTrigger: read(type)
                read(bin_state_y0)
                switch (type) {
                    case 'r':
                    case 0:
                        blocks.push_back(std::make_unique<StrRTrigger>(*this, bin_state_y0));
                        break;
                    case 'f':
                    case 1:
                        blocks.push_back(std::make_unique<StrFTrigger>(*this, bin_state_y0));
                        break;
                    case 'b':
                    case 2:
                        blocks.push_back(std::make_unique<StrBTrigger>(*this, bin_state_y0));
                        break;
                    case 'l':
                    case 3:
                        blocks.push_back(std::make_unique<StrLTrigger>(*this, bin_state_y0));
                        break;
                    default:
                        std::cerr << "Scheme::assign: неверный тип T-триггера\n"
                            << "i: " << i << ", type: " << type << '\n';
                }
                break;

            default:
                std::cerr << "Scheme::assign: при построении схемы использован неизвестный тип блока\n"
                    << "i: " << i << ", block-id: " << int(block_id) << '\n';
        }
    }
    link links;
    read(links)

#undef read
#undef read_arr

    blocks_count = count;
    setAbsoluteLinks(links, reinterpret_cast<const link*>(data));
}
}