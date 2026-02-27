#pragma once

#include <cstdint>

namespace nrcki {
enum class BlockID : uint8_t {
    // Delays (Задержки)
    DelayOn = 1,
    DelayOff,
    DelayOnOff,
    // Dynamic (Динамические)
    Integrator,
    Inertial,
    InertialDifferential,
    Oscillatory,
    StepDelay,
    // Interpolation (Интерполяция)
    PiecewiseLinear,
    // Logical (Логические)
    Or,
    And,
    Xor,
    Not,
    Equal,
    NotEqual,
    Less,
    Greater,
    LessOrEqual,
    GreaterOrEqual,
    // Nonlinear (Нелинейные)
    Saturation,
    Deadband,
    SaturationDeadband,
    Hysteresis,
    HysteresisDeadband,
    LowThreshold,
    HighThreshold,
    VariableHysteresis,
    VariableHysteresisPlus,
    VariableHysteresisMinus,
    // Operators (Операторы)
    Summator,
    Multiplier,
    Divider,
    AbsoluteValue,
    Negate,
    Sign,
    // Pulses (Импульсы)
    RisingPulse,
    FallingPulse,
    ChangePulse,
    Pulse,
    ShortPulse,
    LongPulse,
    DebounceOn,
    DebounceOff,
    DebounceOnOff,
    // Signals (Сигналы)
    ExtInSignal,
    ExtOutSignal,
    IntInSignal,
    IntOutSignal,
    Plot,
    // Sources (Источники)
    Constant,
    Step,
    LinearSource,
    SinusSource,
    // Switches (Переключатели)
    ToggleSwitch,
    // Triggers (Триггеры)
    RsTrigger,
    SrTrigger,
    TTrigger,
    RtsTrigger,
    StrTrigger,
};
}