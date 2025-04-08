#include "managed_windows.hpp"
#include "logger.hpp"
#include "config.hpp"

ManagedWindow::ManagedWindow(Display* display, Window window)
    : display_(display), window_(window)
{
    XSetWindowBorderWidth(display_, window_, wm_constants::BORDER_WIDTH);
    Unfocus();
}

void ManagedWindow::Focus() {
    XSetInputFocus(display_, window_, RevertToPointerRoot, CurrentTime);
    XSetWindowBorder(display_, window_, wm_constants::ACTIVE_BORDER);
    is_focused_ = true;

    Logger::Global().Log("Focused window " + std::to_string(window_), log_level::INFO);
}

void ManagedWindow::Unfocus() {
    XSetWindowBorder(display_, window_, wm_constants::INACTIVE_BORDER);
    is_focused_ = false;
}

void ManagedWindow::MoveResize(int x, int y, int w, int h) {
    XMoveResizeWindow(display_, window_, x, y, w, h);

    Logger::Global().Log(
        "Moved window " + std::to_string(window_) + 
        " to [" + std::to_string(x) + "," + std::to_string(y) +
        "] size " + std::to_string(w) + "x" + std::to_string(h), 
        log_level::INFO);
}

void ManagedWindow::Open() {
    XMapWindow(display_, window_);
    Logger::Global().Log("Mapped window: " + std::to_string(window_), log_level::DEBUG);
}

void ManagedWindow::Close() {
    XDestroyWindow(display_, window_);
    Logger::Global().Log("Closed window: " + std::to_string(window_), log_level::INFO);
} 

void ManagedWindow::SetEventMask(long event_mask) {
    XSelectInput(display_, window_, event_mask);
    Logger::Global().Log("Set event mask " + std::to_string(event_mask) + 
                        " for window " + std::to_string(window_), 
        log_level::DEBUG
    );
}