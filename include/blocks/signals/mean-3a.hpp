#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <algorithm>
#include <sstream>

namespace nrcki {
class Mean3A final : public Block {
    using Type = types::real;

    Ports<Type>* ports;

    // Параметры блока
    Type VZAB;  // Время сглаживания разности
    Type SVZAB; // Отключение сглаживания разности
    Type OG;    // Верхняя граница расхождения
    Type UG;    // Нижняя граница расхождения
    Type ANSF;  // Отключение анализа достоверности по расхождению
    Type ERSW;  // Замещающее значение
    Type FERSW; // Переключение на замещающее значение
    Type SSFU;  // Блокировка сглаживания выходного сигнала
    Type VZSFU; // Время сглаживания выходного сигнала
    Type GWSFU; // Предел скачка выходного сигнала

    // Внутреннее состояние
    mutable Type prev_Y;      // Предыдущее значение Y
    mutable Type prev_diff12; // Предыдущее значение разности X1-X2
    mutable Type prev_diff23; // Предыдущее значение разности X2-X3
    mutable Type prev_diff31; // Предыдущее значение разности X3-X1
    mutable bool initialized; // Флаг инициализации

    // Функция сглаживания (инерционное звено первого порядка)
    Type smooth(Type current, Type prev, Type time_constant) const {
        if (time_constant <= 0)
            return current;
        Type alpha = Context::dt_sec / (time_constant + Context::dt_sec);
        return prev + alpha * (current - prev);
    }

    // Проверка на отклонение с учетом гистерезиса
    bool isDeviation(Type diff) const {
        if (UG == OG)
            return diff > OG; // Простой порог
        // Гистерезис: если разность выше OG или ниже UG
        return (diff > OG) || (diff < UG);
    }

public:
    explicit Mean3A(const Context& context,
                    Type VZAB = 5, Type SVZAB = 1, Type OG    = 0, Type UG = 0,
                    Type ANSF = 0, Type ERSW  = 0, Type FERSW = 0,
                    Type SSFU = 0, Type VZSFU = 5, Type GWSFU = 2.5) :
        Block(context),
        VZAB(VZAB), SVZAB(SVZAB), OG(OG), UG(UG), ANSF(ANSF),
        ERSW(ERSW), FERSW(FERSW), SSFU(SSFU), VZSFU(VZSFU), GWSFU(GWSFU),
        prev_Y(0), prev_diff12(0), prev_diff23(0), prev_diff31(0), initialized(false) {
        ports = new Ports<Type>(6, 8); // 6 входов, 8 выходов
        register_ports(ports);
    }

    void compute() const override {
        Type X1  = *ports->inputs[0];
        Type X2  = *ports->inputs[1];
        Type X3  = *ports->inputs[2];
        Type FG1 = *ports->inputs[3];
        Type FG2 = *ports->inputs[4];
        Type FG3 = *ports->inputs[5];

        // 1. Анализ расхождений между сигналами
        Type diff12 = std::abs(X1 - X2);
        Type diff23 = std::abs(X2 - X3);
        Type diff31 = std::abs(X3 - X1);

        // Сглаживание разностей (если не отключено)
        if (!initialized) {
            prev_diff12 = diff12;
            prev_diff23 = diff23;
            prev_diff31 = diff31;
            initialized = true;
        }

        if (SVZAB == 0) {
            // Сглаживание разрешено
            diff12 = smooth(diff12, prev_diff12, VZAB);
            diff23 = smooth(diff23, prev_diff23, VZAB);
            diff31 = smooth(diff31, prev_diff31, VZAB);
        }

        prev_diff12 = diff12;
        prev_diff23 = diff23;
        prev_diff31 = diff31;

        // Определение превышений порогов
        bool AB12 = isDeviation(diff12);
        bool AB23 = isDeviation(diff23);
        bool AB31 = isDeviation(diff31);

        // Формирование сигналов отклонения по таблице 7.1.4.2
        bool ABWX1_val = (AB31 && AB12) && (FG1 != 0); // X1 неверный
        bool ABWX2_val = (AB12 && AB23) && (FG2 != 0); // X2 неверный
        bool ABWX3_val = (AB23 && AB31) && (FG3 != 0); // X3 неверный
        bool ABW_val   = (AB12 || AB23 || AB31) && (FG1 != 0 || FG2 != 0 || FG3 != 0);

        // 2. Определение достоверности каналов
        bool valid1, valid2, valid3;

        if (ANSF != 0) {
            // Анализ расхождений отключен
            valid1 = (FG1 != 0);
            valid2 = (FG2 != 0);
            valid3 = (FG3 != 0);
        }
        else {
            // Учитываем отклонения
            valid1 = (FG1 != 0) && !ABWX1_val;
            valid2 = (FG2 != 0) && !ABWX2_val;
            valid3 = (FG3 != 0) && !ABWX3_val;
        }

        // 3. Вычисление среднего значения с учетом достоверности
        Type raw_value  = 0;
        int valid_count = 0;
        Type sum        = 0;

        if (valid1) {
            valid_count++;
            sum += X1;
        }
        if (valid2) {
            valid_count++;
            sum += X2;
        }
        if (valid3) {
            valid_count++;
            sum += X3;
        }

        if (valid_count > 0) {
            raw_value = sum / valid_count;
        }
        // Если все недостоверны - raw_value остается 0

        // 4. Применение замещающего значения
        if (FERSW != 0) {
            raw_value = ERSW;
        }

        // 5. Сглаживание выходного сигнала
        if (!initialized) {
            prev_Y = raw_value;
        }

        Type output_Y = raw_value;

        if (SSFU == 0) {
            // Сглаживание разрешено
            Type jump = std::abs(raw_value - prev_Y);
            if (jump > GWSFU) {
                // Применяем инерционное звено только если скачок превышает предел
                output_Y = smooth(raw_value, prev_Y, VZSFU);
            }
        }

        prev_Y            = output_Y;
        ports->outputs[0] = output_Y;

        // 6. Формирование выходных дискретных сигналов
        ports->outputs[1] = (valid_count < 3) ? 1.0 : 0.0;  // S1V3
        ports->outputs[2] = (valid_count < 2) ? 1.0 : 0.0;  // S2V3
        ports->outputs[3] = ABW_val ? 1.0 : 0.0;            // ABW
        ports->outputs[4] = ABWX1_val ? 1.0 : 0.0;          // ABWX1
        ports->outputs[5] = ABWX2_val ? 1.0 : 0.0;          // ABWX2
        ports->outputs[6] = ABWX3_val ? 1.0 : 0.0;          // ABWX3
        ports->outputs[7] = (valid_count >= 2) ? 1.0 : 0.0; // FG
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