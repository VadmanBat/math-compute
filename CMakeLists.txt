cmake_minimum_required(VERSION 3.31)
project(nrcki-scheme VERSION 1.0 LANGUAGES CXX)

# ========== Настройка C++ ==========
set(CMAKE_CXX_STANDARD 23)
set(STATIC_BUILD OFF)

# ========== Зависимости ==========
add_subdirectory(C:/programming/personal-projects/static-libs/algo algo-build)

# ========== Источники и исполняемый файл ==========
include(cmake/source-files.cmake)
add_library(scheme STATIC ${SOURCES})
target_include_directories(scheme PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
add_library(${PROJECT_NAME}::scheme ALIAS scheme)

# ========== Настройки компиляции ==========
#include(cmake/compiler-flags.cmake)

# ========== Сборка примера ==========
if (PROJECT_IS_TOP_LEVEL)
    add_executable(${PROJECT_NAME}-example main.cpp)
    target_link_libraries(${PROJECT_NAME}-example PRIVATE ${PROJECT_NAME}::scheme)
endif ()