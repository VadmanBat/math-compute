if (CMAKE_BUILD_TYPE STREQUAL "Debug") # ========== Отладочные флаги ==========
    target_compile_options(${PROJECT_NAME} PRIVATE
            -O0                            # Отключение оптимизаций для упрощения отладки
            -g                             # Генерация отладочной информации (GDB, LLDB)
            -fno-inline                    # Запрет инлайнинга функций (сохранение стектрейсов)
            -Wall -Wextra -Wpedantic       # Включение дополнительных предупреждений
            -Wshadow                       # Предупреждения о скрытии переменных (одноимённые идентификаторы)
            -Wconversion                   # Предупреждения о неявных числовых преобразованиях
            #-Werror                        # Трактовать предупреждения как ошибки (опционально)

    )
    target_compile_definitions(${PROJECT_NAME} PRIVATE
            DEBUG                          # Активация условной компиляции для отладочного кода
    )
elseif (CMAKE_BUILD_TYPE STREQUAL "Release") # ========== Флаги релиза ==========
    target_compile_options(${PROJECT_NAME} PRIVATE
            -O3                            # Агрессивные оптимизации скорости выполнения
            -funroll-loops                 # Разворот циклов для уменьшения ветвлений
            -fvisibility=hidden            # Скрытие символов по умолчанию (улучшение инкапсуляции в библиотеках)
            -fipa-pta                      # Межпроцедурный анализ указателей (оптимизация работы с памятью)
            -fdevirtualize                 # Замена виртуальных вызовов на статические (оптимизация полиморфизма)
            #-fno-exceptions                # Отключение исключений (если не используются)
            #-fno-rtti                      # Отключение RTTI (если не нужен dynamic_cast/typeid)
            #-march=native                  # Генерация кода под текущий CPU (оптимизация под конкретное железо)
    )
    target_compile_definitions(${PROJECT_NAME} PRIVATE
            NDEBUG                         # Отключение assert() и отладочных проверок
    )
endif()

# ========== Настройка линковщика ==========
set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
)

# ========== Статическая линковка ==========
if (STATIC_BUILD)
    set_target_properties(${PROJECT_NAME} PROPERTIES
            LINK_SEARCH_START_STATIC ON
            LINK_SEARCH_END_STATIC ON
            CXX_VISIBILITY_PRESET hidden
    )
    target_link_options(${PROJECT_NAME} PRIVATE -static)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
            QT_STATICPLUGIN  # Для статической загрузки плагинов Qt
    )
endif()