#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <algorithm>
#include <sstream>

namespace nrcki {
class Mean4A final : public Block {
    using Type = types::real;

    Ports<Type>* ports;

    // Параметры блока
    Type VZ;    // Время инерционного звена для разностей
    Type UG;    // Нижняя граница расхождения
    Type OG;    // Верхняя граница расхождения
    Type ERSW;  // Замещающее значение
    Type SVZ;   // Блокировка времени задержки
    Type ANSF;  // Отключение анализа недостоверности по отклонению
    Type BART;  // Режим работы (0: 3 из 4, 1: 2 из 4)
    Type FERSW; // Подключение замещающего значения

    // Внутреннее состояние для сглаживания разностей
    mutable std::vector<Type> prev_diffs;
    mutable bool initialized;

    // Индексы разностей (6 пар)
    struct DiffPair {
        int i, j;
    };

    static constexpr DiffPair diff_pairs[6] = {
        {0, 1}, {0, 2}, {0, 3}, {1, 2}, {1, 3}, {2, 3}
    };

    // Функция сглаживания
    Type smooth(Type current, Type prev, Type time_constant) const {
        if (time_constant <= 0 || SVZ != 0)
            return current;
        Type alpha = Context::dt_sec / (time_constant + Context::dt_sec);
        return prev + alpha * (current - prev);
    }

    // Проверка на отклонение
    bool isDeviation(Type diff) const {
        return (diff < UG) || (diff > OG);
    }

    // Структура для хранения значений с индексами
    struct ValueWithIndex {
        Type value;
        int index;
        bool valid;

        bool operator<(const ValueWithIndex& other) const {
            return value < other.value;
        }
    };

public:
    explicit Mean4A(const Context& context,
                    Type VZ   = 0, Type UG    = 0, Type OG   = 100,
                    Type ERSW = 0, Type SVZ   = 0, Type ANSF = 0,
                    Type BART = 0, Type FERSW = 0) :
        Block(context),
        VZ(VZ), UG(UG), OG(OG), ERSW(ERSW), SVZ(SVZ),
        ANSF(ANSF), BART(BART), FERSW(FERSW),
        prev_diffs(6, 0), initialized(false) {
        ports = new Ports<Type>(8, 10); // 8 входов, 10 выходов
        register_ports(ports);
    }

    void compute() const override {
        // Получаем входные сигналы
        Type X[4] = {
            *ports->inputs[0],
            *ports->inputs[1],
            *ports->inputs[2],
            *ports->inputs[3]
        };

        Type FG[4] = {
            *ports->inputs[4],
            *ports->inputs[5],
            *ports->inputs[6],
            *ports->inputs[7]
        };

        // 1. Вычисление и сглаживание разностей между всеми парами
        bool exceed_flags[6] = {false};

        if (!initialized) {
            for (int k = 0; k < 6; k++) {
                int i           = diff_pairs[k].i;
                int j           = diff_pairs[k].j;
                Type diff       = std::abs(X[i] - X[j]);
                prev_diffs[k]   = diff;
                exceed_flags[k] = isDeviation(diff);
            }
            initialized = true;
        }
        else {
            for (int k = 0; k < 6; k++) {
                int i              = diff_pairs[k].i;
                int j              = diff_pairs[k].j;
                Type diff          = std::abs(X[i] - X[j]);
                Type smoothed_diff = smooth(diff, prev_diffs[k], VZ);
                prev_diffs[k]      = smoothed_diff;
                exceed_flags[k]    = isDeviation(smoothed_diff);
            }
        }

        // 2. Определение отклонений каналов (ABWX1-ABWX4)
        // Считаем, сколько превышений затрагивает каждый канал
        int exceed_counts[4] = {0, 0, 0, 0};
        for (int k = 0; k < 6; k++) {
            if (exceed_flags[k]) {
                int i = diff_pairs[k].i;
                int j = diff_pairs[k].j;
                exceed_counts[i]++;
                exceed_counts[j]++;
            }
        }

        // Формирование сигналов ABWX
        bool ABWX[4] = {false, false, false, false};
        for (int i = 0; i < 4; i++) {
            // Канал считается отклоняющимся, если участвует в 2+ превышениях
            ABWX[i] = (exceed_counts[i] >= 2) && (FG[i] != 0);
        }

        // 3. Определение достоверности каналов
        bool valid[4];
        for (int i = 0; i < 4; i++) {
            if (ANSF != 0) {
                // Анализ отклонений отключен - используем только FG
                valid[i] = (FG[i] != 0);
            }
            else {
                // Учитываем и FG, и отклонения
                valid[i] = (FG[i] != 0) && !ABWX[i];
            }
        }

        // Подсчет достоверных каналов
        int valid_count = 0;
        for (int i = 0; i < 4; i++) {
            if (valid[i])
                valid_count++;
        }

        // 4. Формирование предварительных сигналов S1V4, S2V4, S3V4
        bool S1V4_pre = (valid_count < 4);
        bool S2V4_pre = (valid_count < 3);
        bool S3V4_pre = (valid_count < 2);

        // 5. Коррекция достоверности в зависимости от режима BART
        bool final_valid[4];
        int final_valid_count = valid_count;

        if (BART != 0) {
            // Режим "2 из 4"
            if (S2V4_pre) {
                // Не менее двух недостоверны
                final_valid_count = 0;
                for (int i = 0; i < 4; i++)
                    final_valid[i] = false;
            }
            else {
                for (int i = 0; i < 4; i++)
                    final_valid[i] = valid[i];
            }
        }
        else {
            // Режим "3 из 4"
            if (S3V4_pre) {
                // Не менее трех недостоверны
                final_valid_count = 0;
                for (int i = 0; i < 4; i++)
                    final_valid[i] = false;
            }
            else {
                for (int i = 0; i < 4; i++)
                    final_valid[i] = valid[i];
            }
        }

        // 6. Вычисление выходного значения
        Type raw_value = 0;

        if (FERSW != 0) {
            // Использование замещающего значения
            raw_value = ERSW;
        }
        else if (final_valid_count > 0) {
            // Вычисление среднего из достоверных каналов
            Type sum = 0;
            for (int i = 0; i < 4; i++) {
                if (final_valid[i]) {
                    sum += X[i];
                }
            }
            raw_value = sum / final_valid_count;
        }
        // Если все недостоверны, raw_value остается 0

        // 7. Формирование выходных сигналов
        ports->outputs[0] = raw_value;                           // Y
        ports->outputs[1] = S1V4_pre ? 1.0 : 0.0;                // S1V4
        ports->outputs[2] = (final_valid_count < 3) ? 1.0 : 0.0; // S2V4
        ports->outputs[3] = (final_valid_count < 2) ? 1.0 : 0.0; // S3V4

        // ABW формируется, если есть хотя бы одно превышение
        bool any_exceed = false;
        for (int k = 0; k < 6; k++) {
            if (exceed_flags[k]) {
                any_exceed = true;
                break;
            }
        }
        ports->outputs[4] = any_exceed ? 1.0 : 0.0; // ABW

        ports->outputs[5] = ABWX[0] ? 1.0 : 0.0; // ABWX1
        ports->outputs[6] = ABWX[1] ? 1.0 : 0.0; // ABWX2
        ports->outputs[7] = ABWX[2] ? 1.0 : 0.0; // ABWX3
        ports->outputs[8] = ABWX[3] ? 1.0 : 0.0; // ABWX4

        // FG (достоверность выхода) - если хотя бы два канала достоверны
        ports->outputs[9] = (final_valid_count >= 2) ? 1.0 : 0.0; // FG
    }

    types::string printSource() const override {
        const auto x1 = CODE_NAME_IN(ports, 0);
        const auto x2 = CODE_NAME_IN(ports, 1);

        const auto y1 = CODE_NAME_OUT(ports, 0);
        const auto y2 = CODE_NAME_OUT(ports, 1);
        const auto y3 = CODE_NAME_OUT(ports, 2);

        std::stringstream line;
        line << y1 << " = (" << x1 << " != 0) || (" << x2 << " != 0);\n";
        line << y2 << " = (" << x1 << " != 0) && (" << x2 << " != 0);\n";
        line << y3 << " = (" << x1 << " != 0) != (" << x2 << " != 0);";
        return line.str();
    }
};
}