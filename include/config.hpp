#pragma once

#include <X11/keysym.h>

namespace wm_constants {
    constexpr const char* LOG_PATH = "/tmp/mywm.log";
    constexpr const char* TERMINAL = "alacritty";
    constexpr const char* LAUNCHER = "dmenu_run";
    
    // Клавиши
    constexpr int CLOSE_WM_KEY = XK_F4;
    constexpr int CLOSE_WIN_KEY = XK_q;
    constexpr int OPEN_WIN_KEY = XK_Return;
    constexpr int OPEN_LAUNCH_KEY = XK_d;
    constexpr unsigned MOD_KEY = Mod4Mask;

    // Окна
    constexpr int BORDER_WIDTH = 2;
    constexpr unsigned long INACTIVE_BORDER = 0x222222;
    constexpr unsigned long ACTIVE_BORDER = 0x00FF00;

    // Тайминги
    constexpr int FOCUS_DELAY_MS = 50;

    // Фреймы
    constexpr unsigned long FRAME_BG_COLOR = 0xFFFFFF;
}

namespace log_level {
    constexpr const char* DEBUG = "DEBUG";
    constexpr const char* ERROR = "ERROR";
    constexpr const char* INFO  = "INFO";
}