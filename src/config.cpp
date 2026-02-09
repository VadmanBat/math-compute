//
// Created by Vadim on 17.05.2025.
//

#include "constants/config.hxx"

#include <vector>

namespace nrcki {
void types::register_all_types() {
    auto& reg = get_type_registry();

    reg[type_hash<bool>()]        = {sizeof(bool), "bool", "bools"};
    reg[type_hash<types::byte>()] = {sizeof(types::byte), "uint8_t", "bytes"};
    reg[type_hash<int>()]         = {sizeof(int), "int", "ints"};
    reg[type_hash<char>()]        = {sizeof(char), "char", "chars"};
    reg[type_hash<types::real>()] = {sizeof(types::real), "double", "reals"};
    reg[type_hash<types::time>()] = {sizeof(types::time), "long long", "times"};

    reg[type_hash<std::vector<bool>>()]        = {sizeof(std::vector<bool>), "std::vector<bool>", "bool_vectors"};
    reg[type_hash<std::vector<types::byte>>()] = {sizeof(std::vector<types::byte>), "std::vector<uint8_t>",
                                                  "byte_vectors"};
    reg[type_hash<types::string>()]            = {sizeof(types::string), "std::string", "strings"};
    reg[type_hash<std::vector<int>>()]         = {sizeof(std::vector<int>), "std::vector<int>", "int_vectors"};
    reg[type_hash<std::vector<types::real>>()] = {sizeof(std::vector<types::real>), "std::vector<double>",
                                                  "real_vectors"};
}

/// Макрос для явной регистрации типа
#define REGISTER_TYPE(T, CODENAME) \
template <> \
const bool types::TypeRegistrar<T>::init = []{ \
const types::size hash = types::type_hash<T>(); \
types::get_type_registry()[hash] = {sizeof(T), #T, #CODENAME}; \
return true; \
}()

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