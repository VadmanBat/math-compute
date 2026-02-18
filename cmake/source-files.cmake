set(CORE_SOURCES
        src/register-types.cpp
)

set(SCHEME_SOURCES
        src/scheme/scheme.cpp
        src/scheme/assign.cpp
        src/scheme/set-links.cpp
        src/scheme/signals.cpp

        src/scheme/create-blocks/delays.cpp
        src/scheme/create-blocks/dynamic.cpp
        src/scheme/create-blocks/pulses.cpp
        src/scheme/create-blocks/interpolation.cpp
        src/scheme/create-blocks/switches.cpp
        src/scheme/create-blocks/logical.cpp
        src/scheme/create-blocks/nonlinear.cpp
        src/scheme/create-blocks/operators.cpp
        src/scheme/create-blocks/sources.cpp
        src/scheme/create-blocks/triggers.cpp
)

set(BLOCK_SOURCES
        src/block/block.cpp
)

set(SOURCES
        ${CORE_SOURCES}
        ${SCHEME_SOURCES}
        ${BLOCK_SOURCES}
)