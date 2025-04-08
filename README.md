# MyWM - Минималистичный X11 Оконный Менеджер

## 📝 Описание

MyWM — это легковесный оконный менеджер для X11, написанный на C++ с использованием парадигм ООП. Проект создан для обучения и демонстрации принципов работы оконных менеджеров.

## ✨ Особенности

- 🧩 Модульная архитектура (WindowManager, ManagedWindow, Logger)
- 🖱️ Поддержка базовых операций (перемещение, фокусировка)
- ⌨️ Горячие клавиши (Win+Enter - новое окно, Win+Q - закрыть окно)
- 📊 Простой тайлинг (вертикальное разделение)
- 📝 Логирование в файл (`/tmp/mywm.log`)

## 🛠️ Зависимости

- Компилятор C++17 (g++/clang)
- Xlib (libx11-dev)
- CMake (>= 3.10)

*Для тестирования:*
- Xephyr (xserver-xephyr)

## 🚀 Сборка и Установка

```bash
# Клонировать репозиторий
git clone https://github.com/Zestria/my_wm.git
cd my_wm

# Сборка
mkdir build && cd build
cmake ..
make

# Запуск в Xephyr (рекомендуется для тестирования)
Xephyr :1 -ac -br -noreset -screen 1280x720 &
DISPLAY=:1 ./my_wm
```

## 📂 Структура проекта
mywm/
├── include/               # Заголовочные файлы
│   ├── window_manager.hpp
│   ├── managed_window.hpp
│   └── ...
├── src/                   # Исходный код
│   ├── main.cpp
│   ├── window_manager.cpp
│   └── ...
├── CMakeLists.txt         # Конфигурация сборки
└── README.md              # Этот файл