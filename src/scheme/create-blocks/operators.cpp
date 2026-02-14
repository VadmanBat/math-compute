#include "nrcki/scheme.h"

#include "blocks/operators/summator.hpp"
#include "blocks/operators/multiplier.hpp"
#include "blocks/operators/divider.hpp"
#include "blocks/operators/absolute-value.hpp"
#include "blocks/operators/sign.hpp"

namespace nrcki {
using types::real;
/**
 * Добавляет "сумматор" на схему.
 * @param n количество переменных для сложения
 * @param coefficients весовые множители
 */
void Scheme::addSummator(int n, double* coefficients) {
    ++blocks_count;
    blocks.push_back(std::make_unique <Summator>(*this, std::vector <real>(coefficients, coefficients + n)));
}

/**
 * Добавляет блок перемножения сигналов на схему.
 * Блок вычисляет произведение всех входных сигналов.
 * @param n количество входных сигналов для перемножения
 */
void Scheme::addMultiplier(int n) {
    ++blocks_count;
    blocks.push_back(std::make_unique <Multiplier>(*this, n));
}

/**
 * Добавляет блок деления сигналов на схему.
 * Блок вычисляет частное двух входных сигналов: dividend / divisor.
 * @param value_if_div_null значение, возвращаемое при делении на ноль
 */
void Scheme::addDivider(double value_if_div_null) {
    ++blocks_count;
    blocks.push_back(std::make_unique <Divider>(*this, value_if_div_null));
}

/**
 * Добавляет блок вычисления абсолютного значения на схему.
 * Блок возвращает модуль входного сигнала.
 */
void Scheme::addAbsoluteValue() {
    ++blocks_count;
    blocks.push_back(std::make_unique <AbsoluteValue>(*this));
}

/**
 * Добавляет блок определения знака сигнала на схему.
 * Блок возвращает:
 * <ul>
 * <li>для отрицательного входного сигнала (y < 0): выход = -1;</li>
 * <li>для нулевого входного сигнала (y = 0): выход = 0;</li>
 * <li>для положительного входного сигнала (y > 0): выход = +1.</li>
 * </ul>
 */
void Scheme::addSign() {
    ++blocks_count;
    blocks.push_back(std::make_unique <Sign>(*this));
}
}