#pragma once

#include <X11/keysym.h>

namespace wm_constants {
    // Paths and Applications
    constexpr const char* LOG_PATH = "/tmp/mywm.log";
    constexpr const char* TERMINAL = "alacritty";
    constexpr const char* LAUNCHER = "dmenu_run";
    
    // Key Bindings (Mod = Super/Windows key)
    constexpr int CLOSE_WM_KEY = XK_F4;      // Mod+F4
    constexpr int CLOSE_WIN_KEY = XK_q;      // Mod+Q
    constexpr int OPEN_WIN_KEY = XK_Return;  // Mod+Enter
    constexpr int OPEN_LAUNCH_KEY = XK_d;    // Mod+D
    constexpr unsigned MOD_KEY = Mod4Mask;   // Modifier key
    
    // Window Decoration
    constexpr int BORDER_WIDTH = 2;                     // Frame border
    constexpr unsigned long INACTIVE_BORDER = 0x222222; // Dark gray
    constexpr unsigned long ACTIVE_BORDER = 0x00FF00;   // Bright green
    
    // Timing
    constexpr int FOCUS_DELAY_MS = 50;
    
    // Frame Appearance
    constexpr unsigned long FRAME_BG_COLOR = 0xFFFFFF;
}

namespace log_level {
    constexpr const char* DEBUG = "DEBUG";
    constexpr const char* ERROR = "ERROR";
    constexpr const char* INFO  = "INFO";
}