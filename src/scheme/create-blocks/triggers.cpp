#include "nrcki/scheme.h"

#include "blocks/triggers/rs-trigger.hpp"
#include "blocks/triggers/sr-trigger.hpp"

#include "blocks/triggers/t-triggers/rts-r-trigger.hpp"
#include "blocks/triggers/t-triggers/rts-f-trigger.hpp"
#include "blocks/triggers/t-triggers/rts-b-trigger.hpp"
#include "blocks/triggers/t-triggers/rts-l-trigger.hpp"

#include "blocks/triggers/t-triggers/str-r-trigger.hpp"
#include "blocks/triggers/t-triggers/str-f-trigger.hpp"
#include "blocks/triggers/t-triggers/str-b-trigger.hpp"
#include "blocks/triggers/t-triggers/str-l-trigger.hpp"

namespace nrcki {
/**
 * Добавляет RS-триггер (Reset-Set) на схему.
 * Триггер с приоритетом сброса (Reset доминирует).
 *
 * Входы:
 *   - Вход 0: S (установка)
 *   - Вход 1: R (сброс)
 *
 * Таблица истинности:
 * <table>
 * <tr><th>R</th><th>S</th><th>Q</th><th>Состояние</th></tr>
 * <tr><td>0</td><td>0</td><td>Q<sub>prev</sub></td><td>Хранение</td></tr>
 * <tr><td>0</td><td>1</td><td>1</td><td>Установка</td></tr>
 * <tr><td>1</td><td>0</td><td>0</td><td>Сброс</td></tr>
 * <tr><td>1</td><td>1</td><td>0</td><td>Сброс (приоритет)</td></tr>
 * </table>
 *
 * Применение:
 * <ul>
 *   <li>запоминание состояний в системах управления;</li>
 *   <li>сигнализация аварийных состояний;</li>
 *   <li>управление режимами работы оборудования.</li>
 * </ul>
 *
 * @param y0 начальное состояние триггера
 */
void Scheme::addRsTrigger(bool y0) {
    ++blocks_count;
    blocks.push_back(std::make_unique<RsTrigger>(*this, y0));
}

/**
 * Добавляет SR-триггер (Set-Reset) на схему.
 * Триггер с приоритетом установки (Set доминирует).
 *
 * Входы:
 *   - Вход 0: S (установка)
 *   - Вход 1: R (сброс)
 *
 * Таблица истинности:
 * <table>
 * <tr><th>S</th><th>R</th><th>Q</th><th>Состояние</th></tr>
 * <tr><td>0</td><td>0</td><td>Q<sub>prev</sub></td><td>Хранение</td></tr>
 * <tr><td>0</td><td>1</td><td>0</td><td>Сброс</td></tr>
 * <tr><td>1</td><td>0</td><td>1</td><td>Установка</td></tr>
 * <tr><td>1</td><td>1</td><td>1</td><td>Установка (приоритет)</td></tr>
 * </table>
 *
 * Применение:
 * <ul>
 *   <li>системы с приоритетом активации;</li>
 *   <li>управление критически важными процессами;</li>
 *   <li>защитные цепи с приоритетом включения.</li>
 * </ul>
 *
 * @param y0 начальное состояние триггера
 */
void Scheme::addSrTrigger(bool y0) {
    ++blocks_count;
    blocks.push_back(std::make_unique<SrTrigger>(*this, y0));
}

/**
 * Добавляет специальный T-триггер (Toggle) с синхронным входом на схему.
 *
 * Имеет 3 входа:
 *   - Вход 0: S (установка)
 *   - Вход 1: T (переключение)
 *   - Вход 2: R (сброс)
 *
 * Логика работы:
 *   - Если S=1 или R=1: работает как RS-триггер (с приоритетом сброса)
 *   - Если S=0 и R=0: работает как классический T-триггер:
 *        <table>
 *        <tr><th>Тип</th><th>Срабатывание</th><th>Поведение</th></tr>
 *        <tr><td>'r'</td><td>Передний фронт T</td><td>Q = !Q<sub>prev</sub></td></tr>
 *        <tr><td>'f'</td><td>Задний фронт T</td><td>Q = !Q<sub>prev</sub></td></tr>
 *        <tr><td>'b'</td><td>Любой фронт T</td><td>Q = !Q<sub>prev</sub></td></tr>
 *        <tr><td>'l'</td><td>Уровень T=1</td><td>Непрерывно инвертирует Q</td></tr>
 *        </table>
 *
 * Применение:
 * <ul>
 *   <li>сложные системы управления с режимами переключения;</li>
 *   <li>комбинированные схемы управления состоянием;</li>
 *   <li>универсальные триггерные системы.</li>
 * </ul>
 *
 * @param type тип срабатывания T-режима ('r', 'f', 'b', 'l')
 * @param y0 начальное состояние триггера
 */
void Scheme::addRtsTrigger(char type, bool y0) {
    ++blocks_count;
    switch (type) {
        case 'r':
            blocks.push_back(std::make_unique<RtsRTrigger>(*this, y0));
            break;
        case 'f':
            blocks.push_back(std::make_unique<RtsFTrigger>(*this, y0));
            break;
        case 'b':
            blocks.push_back(std::make_unique<RtsBTrigger>(*this, y0));
            break;
        case 'l':
            blocks.push_back(std::make_unique<RtsLTrigger>(*this, y0));
            break;
    }
}

/**
 * Добавляет специальный T-триггер (Toggle) с асинхронным входом на схему.
 *
 * Имеет 3 входа:
 *   - Вход 0: S (установка)
 *   - Вход 1: T (переключение)
 *   - Вход 2: R (сброс)
 *
 * Логика работы:
 *   - Если S=1 или R=1: работает как RS-триггер (с приоритетом сброса)
 *   - Если S=0 и R=0: работает как классический T-триггер:
 *        <table>
 *        <tr><th>Тип</th><th>Срабатывание</th><th>Поведение</th></tr>
 *        <tr><td>'r'</td><td>Передний фронт T</td><td>Q = !Q<sub>prev</sub></td></tr>
 *        <tr><td>'f'</td><td>Задний фронт T</td><td>Q = !Q<sub>prev</sub></td></tr>
 *        <tr><td>'b'</td><td>Любой фронт T</td><td>Q = !Q<sub>prev</sub></td></tr>
 *        <tr><td>'l'</td><td>Уровень T=1</td><td>Непрерывно инвертирует Q</td></tr>
 *        </table>
 *
 * Применение:
 * <ul>
 *   <li>системы реального времени с жесткими ограничениями;</li>
 *   <li>обработка критичных событий без задержек;</li>
 *   <li>аварийные системы управления.</li>
 * </ul>
 *
 * @param type тип срабатывания T-режима ('r', 'f', 'b', 'l')
 * @param y0 начальное состояние триггера
 */
void Scheme::addStrTrigger(char type, bool y0) {
    ++blocks_count;
    switch (type) {
        case 'r':
            blocks.push_back(std::make_unique<StrRTrigger>(*this, y0));
            break;
        case 'f':
            blocks.push_back(std::make_unique<StrFTrigger>(*this, y0));
            break;
        case 'b':
            blocks.push_back(std::make_unique<StrBTrigger>(*this, y0));
            break;
        case 'l':
            blocks.push_back(std::make_unique<StrLTrigger>(*this, y0));
            break;
    }
}
}