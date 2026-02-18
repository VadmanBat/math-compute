#pragma once

#include "constants/config.hxx"

#include "block.h"
#include "ports.hpp"

#include <cmath>
#include <sstream>
#include <queue>

namespace nrcki {
class ReplaceInvalidSignal final : public Block {
    using Type = types::real;
    using Time = types::time;

    Ports<Type>* ports;
    const Type TW1;

    mutable std::queue<Type> delay_buffer; // Буфер задержки на 3 цикла
    mutable Time timer_start;              // Время начала задержки TW1
    mutable bool in_transition;            // Флаг перехода на нормальный режим
    mutable Type prev_KFGV;                // Предыдущее значение KFGV
    mutable Type prev_Y;                   // Предыдущее значение Y

public:
    explicit ReplaceInvalidSignal(const Context& context, const Type& TW1) :
        Block(context),
        TW1(static_cast<Time>(TW1 * Context::sec)),
        timer_start(std::numeric_limits<Time>::max()),
        in_transition(false),
        prev_KFGV(1), // Начальное состояние - достоверно
        prev_Y(0) {
        ports = new Ports<Type>(4, 2);
        register_ports(ports);

        for (int i = 0; i < 3; ++i) {
            delay_buffer.push(0);
        }
    }

    void compute() const override {
        Type ERSW = *ports->inputs[0]; // Замещающее значение
        Type X    = *ports->inputs[1]; // Входной сигнал
        Type KFG  = *ports->inputs[2]; // Достоверность (1-достоверно, 0-недостоверно)
        Type KFGR = *ports->inputs[3]; // Принудительная недостоверность

        // Обновление буфера задержки
        delay_buffer.push(X);
        if (delay_buffer.size() > 3) {
            delay_buffer.pop();
        }
        Type delayed_X = delay_buffer.front(); // Сигнал X, задержанный на 3 цикла

        // Основная логика работы блока
        if (KFG != 0) {
            // Сигнал достоверен
            if (KFGR == 0) {
                // Нет принудительной недостоверности
                if (!in_transition) {
                    // Начало перехода на нормальный режим
                    timer_start   = context.time;
                    in_transition = true;
                }

                if (context.time >= timer_start + TW1) {
                    // Переход завершен - нормальный режим
                    ports->outputs[0] = delayed_X; // Y = X задержанный на 3 цикла
                    ports->outputs[1] = 1;         // KFGV = 1 (достоверно)
                    prev_KFGV         = 1;
                }
                else {
                    // В процессе перехода
                    ports->outputs[0] = ERSW; // Y = замещающее значение
                    ports->outputs[1] = 0;    // KFGV = 0 (недостоверно)
                }
            }
            else {
                // Принудительная недостоверность
                ports->outputs[0] = ERSW; // Y = замещающее значение
                ports->outputs[1] = 0;    // KFGV = 0 (недостоверно)
                in_transition     = false;
                prev_KFGV         = 0;
            }
        }
        else {
            // Сигнал недостоверен
            ports->outputs[0] = ERSW; // Y = замещающее значение
            if (KFGR == 0) {
                ports->outputs[1] = prev_KFGV; // KFGV сохраняет предыдущее значение
            }
            else {
                ports->outputs[1] = 0; // KFGV = 0 по принуждению
                prev_KFGV         = 0;
            }

            in_transition = false; // Сброс перехода
        }

        prev_Y = ports->outputs[0];
    }

    types::string printMemory() const override {
        const auto time_type = CODE_NAME_TYPE(Time);
        const auto type      = CODE_NAME_TYPE(Type);
        const auto max_time  = CODE_NAME_MAX(Time);

        REGISTER_VAR(delay_buffer)
        REGISTER_VAR(timer_start)
        REGISTER_VAR(in_transition)
        REGISTER_VAR(prev_KFGV)
        REGISTER_VAR(prev_Y)

        std::stringstream line;
        line << "std::queue<" << type << "> " << var_delay_buffer << "({0, 0, 0});\n";
        line << time_type << " " << var_timer_start << " = " << max_time << ";\n";
        line << "bool " << var_in_transition << " = false;\n";
        line << type << " " << var_prev_KFGV << " = 1;\n";
        line << type << " " << var_prev_Y << " = 0;";
        return line.str();
    }

    types::string printSource() const override {
        const auto ERSW = CODE_NAME_IN(ports, 0);
        const auto X    = CODE_NAME_IN(ports, 1);
        const auto KFG  = CODE_NAME_IN(ports, 2);
        const auto KFGR = CODE_NAME_IN(ports, 3);
        const auto Y    = CODE_NAME_OUT(ports, 0);
        const auto KFGV = CODE_NAME_OUT(ports, 1);

        REGISTER_VAR(delay_buffer)
        REGISTER_VAR(timer_start)
        REGISTER_VAR(in_transition)
        REGISTER_VAR(prev_KFGV)
        REGISTER_VAR(prev_Y)

        std::stringstream line;
        line << var_delay_buffer << ".push(" << X << ");\n";
        line << "if (" << var_delay_buffer << ".size() > 3) " << var_delay_buffer << ".pop();\n";
        line << "auto delayed_X = " << var_delay_buffer << ".front();\n\n";

        line << "if (" << KFG << " != 0) {\n";
        line << "    if (" << KFGR << " == 0) {\n";
        line << "        if (!" << var_in_transition << ") {\n";
        line << "            " << var_timer_start << " = time;\n";
        line << "            " << var_in_transition << " = true;\n";
        line << "        }\n";
        line << "        if (time >= " << var_timer_start << " + " << TW1 << ") {\n";
        line << "            " << Y << " = delayed_X;\n";
        line << "            " << KFGV << " = 1;\n";
        line << "            " << var_prev_KFGV << " = 1;\n";
        line << "        } else {\n";
        line << "            " << Y << " = " << ERSW << ";\n";
        line << "            " << KFGV << " = 0;\n";
        line << "        }\n";
        line << "    } else {\n";
        line << "        " << Y << " = " << ERSW << ";\n";
        line << "        " << KFGV << " = 0;\n";
        line << "        " << var_in_transition << " = false;\n";
        line << "        " << var_prev_KFGV << " = 0;\n";
        line << "    }\n";
        line << "} else {\n";
        line << "    " << Y << " = " << ERSW << ";\n";
        line << "    if (" << KFGR << " == 0) {\n";
        line << "        " << KFGV << " = " << var_prev_KFGV << ";\n";
        line << "    } else {\n";
        line << "        " << KFGV << " = 0;\n";
        line << "        " << var_prev_KFGV << " = 0;\n";
        line << "    }\n";
        line << "    " << var_in_transition << " = false;\n";
        line << "}\n";
        line << var_prev_Y << " = " << Y << ";";

        return line.str();
    }
};
}