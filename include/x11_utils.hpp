#pragma once

#include <X11/Xlib.h>
#include <logger.hpp>
#include <config.hpp>

#include "managed_windows.hpp"
#include "config.hpp"
#include "logger.hpp"

namespace x11_utils {

class X11ErrorHandler {
public:
    static int Handle(Display* display, XErrorEvent* ev) {
        char buff[256];
        XGetErrorText(ev->display, ev->error_code, buff, sizeof(buff));
        Logger::Global().Log(std::string(buff), log_level::ERROR);
        return 0;
    }
};

class EventProcessor {
public:
    static KeySym GetKeysymFromEvent(const XKeyEvent& e) {
        XKeyEvent tmp = e;
        return XLookupKeysym(&tmp, 0);
    }
};

class WindowFinder {
private:
    Window target_;
public:
    explicit WindowFinder(Window target) : target_(target) {}

    bool operator()(const ManagedWindow& w) const {
        return w.GetX11Window() == target_;
    }
    static auto MakeFinder(Window target) {
        return [target](const ManagedWindow& w) {
            return w.GetX11Window() == target;
        };
    }
};

}