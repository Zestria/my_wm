#pragma once

#include <X11/keysym.h>

namespace wm_constants {
    constexpr const char* LOG_PATH = "/tmp/vm.log";
    
    // Клавиши
    constexpr int CLOSE_KEY = XK_F4;
    constexpr unsigned MOD_KEY = Mod4Mask;

    // Окна
    constexpr int BORDER_WIDTH = 2;
    constexpr unsigned long INACTIVE_BORDER = 0x222222;
    constexpr unsigned long ACTIVE_BORDER = 0x00FF00;

    // Тайминги
    constexpr int FOCUS_DELAY_MS = 50;
}

namespace log_level {
    constexpr const char* DEBUG = "DEBUG";
    constexpr const char* ERROR = "ERROR";
    constexpr const char* INFO  = "INFO";
}