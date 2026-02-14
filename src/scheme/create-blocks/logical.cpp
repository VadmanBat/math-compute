#include "nrcki/scheme.h"

#include "blocks/logical/and.hpp"
#include "blocks/logical/or.hpp"
#include "blocks/logical/not.hpp"
#include "blocks/logical/equal.hpp"
#include "blocks/logical/not-equal.hpp"
#include "blocks/logical/less.hpp"
#include "blocks/logical/less-or-equal.hpp"
#include "blocks/logical/greater.hpp"
#include "blocks/logical/greater-or-equal.hpp"

namespace nrcki {
/**
 * Добавляет логический блок "И" (AND, &&) на схему.
 * Блок выполняет операцию логического умножения:
 *   Возвращает TRUE, если ВСЕ входные сигналы != 0,
 *   иначе возвращает FALSE.
 *
 * Для оптимизации вычислений рекомендуется:
 *   Располагать входные сигналы в порядке УБЫВАНИЯ вероятности
 *   появления ложного сигнала (0). Это позволяет прерывать
 *   вычисления при первом же ложном входе.
 *
 * @param n количество входных сигналов (больше 1)
 */
void Scheme::addAnd(int n) {
    ++blocks_count;
    blocks.push_back(std::make_unique <And>(*this, n));
}

/**
 * Добавляет логический блок "ИЛИ" (OR, ||) на схему.
 * Блок выполняет операцию логического сложения:
 *   Возвращает TRUE, если ХОТЯ БЫ ОДИН входной сигнал != 0,
 *   иначе возвращает FALSE.
 *
 * Для оптимизации вычислений рекомендуется:
 *   Располагать входные сигналы в порядке УБЫВАНИЯ вероятности
 *   появления истинного сигнала (1). Это позволяет прерывать
 *   вычисления при первом же истинном входе.
 *
 * @param n количество входных сигналов (больше 1)
 */
void Scheme::addOr(int n) {
    ++blocks_count;
    blocks.push_back(std::make_unique <Or>(*this, n));
}

/**
 * Добавляет логический блок "НЕ" (NOT, !) на схему.
 * Блок выполняет операцию логического отрицания:
 *   Возвращает TRUE, если входной сигнал = FALSE,
 *   иначе возвращает FALSE.
 */
void Scheme::addNot() {
    ++blocks_count;
    blocks.push_back(std::make_unique <Not>(*this));
}

/**
 * Добавляет блок сравнения "Равно" (==) на схему.
 * Блок сравнивает два входных сигнала:
 *   Возвращает TRUE, если |A - B| < ε (с учётом машинной точности),
 *   иначе возвращает FALSE.
 */
void Scheme::addEqual() {
    ++blocks_count;
    blocks.push_back(std::make_unique <Equal>(*this));
}

/**
 * Добавляет блок сравнения "Не равно" (!=) на схему.
 * Блок сравнивает два входных сигнала:
 *   Возвращает TRUE, если |A - B| >= ε (с учётом машинной точности),
 *   иначе возвращает FALSE.
 */
void Scheme::addNotEqual() {
    ++blocks_count;
    blocks.push_back(std::make_unique <NotEqual>(*this));
}

/**
 * Добавляет блок сравнения "Меньше" (<) на схему.
 * Блок сравнивает два входных сигнала:
 *   Возвращает TRUE, если A < B,
 *   иначе возвращает FALSE.
 */
void Scheme::addLess() {
    ++blocks_count;
    blocks.push_back(std::make_unique <Less>(*this));
}

/**
 * Добавляет блок сравнения "Меньше или равно" (<=) на схему.
 * Блок сравнивает два входных сигнала:
 *   Возвращает TRUE, если A <= B,
 *   иначе возвращает FALSE.
 */
void Scheme::addLessOrEqual() {
    ++blocks_count;
    blocks.push_back(std::make_unique <LessOrEqual>(*this));
}

/**
 * Добавляет блок сравнения "Больше" (>) на схему.
 * Блок сравнивает два входных сигнала:
 *   Возвращает TRUE, если A > B,
 *   иначе возвращает FALSE.
 */
void Scheme::addGreater() {
    ++blocks_count;
    blocks.push_back(std::make_unique <Greater>(*this));
}

/**
 * Добавляет блок сравнения "Больше или равно" (>=) на схему.
 * Блок сравнивает два входных сигнала:
 *   Возвращает TRUE, если A >= B,
 *   иначе возвращает FALSE.
 */
void Scheme::addGreaterOrEqual() {
    ++blocks_count;
    blocks.push_back(std::make_unique <GreaterOrEqual>(*this));
}
}