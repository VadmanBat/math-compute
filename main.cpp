#include "nrcki/scheme.h"

int main() {
    nrcki::types::register_all_types();

    /*constexpr nrcki::types::link n = 4;
    const nrcki::types::link absolute_links[2 * n] = { 0, 0, 1, 2, 2, 1, 2, 3 };

    double coeffs[2] = {1, -1};
    double c1[2] = {1};

    nrcki::Scheme scheme;
    scheme.addConstant();
    scheme.addSummator(2, coeffs);
    scheme.addOscillatory();
    //scheme.addSummator(1, c1);
    scheme.setAbsoluteLinks(n-1, absolute_links);*/

    constexpr nrcki::types::link n = 4;
    const nrcki::types::link absolute_links[2 * n] = { 0, 0, 1, 1, 2, 2, 3, 3 };

    nrcki::Scheme scheme;
    scheme.addLinearSource();
    scheme.addStepDelay();
    scheme.addStepDelay();
    scheme.addStepDelay();
    scheme.addStepDelay();
    scheme.setAbsoluteLinks(n, absolute_links);

    scheme.printOutputPorts<double>();

    for (int i = 0; i < 100; ++i) {
        std::cout << "i = " << i << '\n';
        scheme.compute(1);
        scheme.printOutputPorts<double>();
    }

    return 0;
}

/**
 *
 * Вопросы:
 *  - работа блока "временное подтверждение";
 *  - инициализация и реализация debounce (необходимо утвердить);
 *
 * Текущие задачи:
 *  - добавить внешних сигналов;
 *  - добавить заморозку связи;
 *
 *  - builder (загрузка сигналов, параллельный расчёт, компиляция);
 *  - solver (разработка)
 *
 * Неприоритетные задачи:
 *  - корректировать описание блоков;
 *  - доделать проверку констант
 *
 *  - вынести логику выделения памяти в родительский класс схемы;
 *
 *  - блоки на добавление: производные по формулам Ньютона (нет шума), по МНК (шум), ОГР;
 *  - поддержка инициализации сложных типов портов в схеме;
 *  - перенести логику кодогенерации в отдельные cpp-файлы;
 *
 *  - проверить ограничения гистерезиса
 *
 * В кодогенерации менять код:
 *  - при умножении на 0 или 1;
 *  - при сложении/вычитании 0;
 *  - при делении на 1
 * */