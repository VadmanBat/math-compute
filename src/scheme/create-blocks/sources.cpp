#include "../../../include/nrcki/scheme.h"

#include "blocks/sources/constant.hpp"
#include "blocks/sources/step.hpp"
#include "blocks/sources/linear-source.hpp"
#include "blocks/sources/sinus-source.hpp"

namespace nrcki {
/**
 * Добавляет источник постоянного сигнала на схему.
 * Блок генерирует неизменный во времени сигнал заданной величины.
 * @param c значение постоянного сигнала
 */
void Scheme::addConstant(double c) {
    ++blocks_count;
    blocks.push_back(std::make_unique <Constant>(*this, c));
}

/**
 * Добавляет источник ступенчатого сигнала на схему.
 * Генерирует сигнал, который скачкообразно изменяет значение в заданный момент времени:
 *   - для времени t < Time: выход = y0;
 *   - для времени t >= Time: выход = yk.
 *
 * @param time момент времени скачка [сек]
 * @param yk установившееся значение после скачка
 * @param y0 начальное значение до скачка
 */
void Scheme::addStep(double time, double yk, double y0) {
    ++blocks_count;
    blocks.push_back(std::make_unique <Step>(*this, time, yk, y0));
}

/**
 * Добавляет источник линейно изменяющегося сигнала на схему.
 * Генерирует сигнал, линейно зависящий от времени: y = k*t + b,
 * где t - текущее время моделирования.
 *
 * @param k коэффициент наклона (скорость изменения)
 * @param b начальное значение при t=0
 */
void Scheme::addLinearSource(double k, double b) {
    ++blocks_count;
    blocks.push_back(std::make_unique <LinearSource>(*this, k, b));
}

/**
 * Добавляет источник синусоидального сигнала на схему.
 * Генерирует гармонический сигнал по формуле:
 *   y = A * sin(ω*t + φ)
 *
 * @param a амплитуда сигнала (A)
 * @param w угловая скорость [рад/сек] (ω)
 * @param f начальная фаза [рад] (φ)
 */
void Scheme::addSinusSource(double a, double w, double f) {
    ++blocks_count;
    blocks.push_back(std::make_unique <SinusSource>(*this, a, w, f));
}
}