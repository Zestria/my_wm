#pragma once

#include <X11/Xlib.h>
#include <logger.hpp>
#include <config.hpp>

#include "managed_windows.hpp"
#include "config.hpp"
#include "logger.hpp"

namespace x11_utils {

/*
 * X11 Error Handler
 * Converts X11 error codes to human-readable messages and logs them
 */
class X11ErrorHandler {
public:
    static int Handle(Display* display, XErrorEvent* ev) {
        char buff[256];
        XGetErrorText(ev->display, ev->error_code, buff, sizeof(buff));
        Logger::Global().Log(std::string(buff), log_level::ERROR);
        return 0;
    }
};

/*
 * X11 Event Processor
 * Provides utilities for working with X11 events
 */
class EventProcessor {
public:
    static KeySym GetKeysymFromEvent(const XKeyEvent& e) {
        XKeyEvent tmp = e; // Copy needed because XLookupKeysym may modify
        return XLookupKeysym(&tmp, 0); // Get keysym from keycode
    }
};

/*
 * Window Finder Utility
 * Helps locate managed windows in containers
 */
class WindowFinder {
public:
    // Creates a predicate function to find windows by frame ID
    static auto MakeFinder(Window target) {
        return [target](const ManagedWindow& w) {
            return w.GetFrame() == target;
        };
    }
};

}