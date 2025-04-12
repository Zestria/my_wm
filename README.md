# MyWM - Минималистичный X11 Оконный Менеджер

## 📝 Описание

MyWM — это легковесный оконный менеджер для X11, написанный на C++ с использованием парадигм ООП.

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
