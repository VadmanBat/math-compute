#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <unordered_map>

/// Используемые типы данных
namespace nrcki::types {
using byte = uint8_t;     // Байт (8 бит)
using size = std::size_t; // Размер: индексация
using real = double;      // Вещественный: вычисления

using time = long long; // Время: целочисленное определение времени
using link = uint32_t;  // Связь: индексация связей блоков

using string = std::string; // Строка

/// Информация о типе
struct TypeInfo {
    size size = -1;
    string name, codename;
};

/// Глобальная база данных типов
inline constexpr auto& get_type_registry() {
    static std::unordered_map<size, TypeInfo> registry;
    return registry;
}

/// Шаблон для регистрации типов
template <typename T>
struct TypeRegistrar {
    static const bool init; // Триггер инициализации
};

/// Получить хеш типа
template <typename T>
inline size type_hash() {
    return typeid(T).hash_code();
}

/// Получить размер типа по хешу
inline size type_size(const size hash) {
    const auto& reg = get_type_registry();
    if (const auto it = reg.find(hash); it != reg.end())
        return it->second.size;
    throw std::runtime_error("type_size: unknown type hash");
}

/// Получить имя типа по хешу
inline string type_name(const size hash) {
    const auto& reg = get_type_registry();
    if (const auto it = reg.find(hash); it != reg.end())
        return it->second.name;
    throw std::runtime_error("type_name: unknown type hash");
}

/// Получить имя типа данных в коде по хешу
inline string type_codename(const size hash) {
    const auto& reg = get_type_registry();
    if (const auto it = reg.find(hash); it != reg.end())
        return it->second.codename;
    throw std::runtime_error("type_codename: unknown type hash");
}


void register_all_types();
}
