//
// Created by Vadim on 17.05.2025.
//

#include "constants/config.hxx"

#include <vector>

namespace nrcki {
void types::register_all_types() {
    auto& reg = get_type_registry();

    /// Макрос для явной регистрации типа
#define REGISTER_TYPE(T, CODENAME) reg[type_hash<T>()] = {sizeof(T), #T, #CODENAME}

    /// Явная регистрация стандартных типов
    REGISTER_TYPE(bool, bools);
    REGISTER_TYPE(types::byte, bytes);
    REGISTER_TYPE(int, ints);
    REGISTER_TYPE(char, chars);
    REGISTER_TYPE(types::real, reals);
    REGISTER_TYPE(types::time, times);

    REGISTER_TYPE(std::vector <bool>, bool_vectors);
    REGISTER_TYPE(std::vector <types::byte>, byte_vectors);
    REGISTER_TYPE(types::string, strings);
    REGISTER_TYPE(std::vector <int>, int_vectors);
    REGISTER_TYPE(std::vector <types::real>, real_vectors);

#undef REGISTER_TYPE
}
}