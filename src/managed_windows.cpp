#include "managed_windows.hpp"
#include "logger.hpp"
#include "config.hpp"

ManagedWindow::ManagedWindow(Display* display, Window window)
    : display_(display), window_(window), frame_(0), is_framed_(false)
{
    XSetWindowBorderWidth(display_, window_, wm_constants::BORDER_WIDTH);
    Unfocus();
}

ManagedWindow::~ManagedWindow() {
    if(is_framed_) {
        XUnmapWindow(display_, frame_);
        XReparentWindow(display_, window_, XRootWindow(display_, XDefaultScreen(display_)), 0, 0);
        XDestroyWindow(display_, frame_);
    }

    Logger::Global().Log("Destroyed ManagedWindow for X11 window " + std::to_string(window_),
                        log_level::DEBUG);
}

void ManagedWindow::CreateFrame(Window root) {
    if(is_framed_) return;

    XWindowAttributes attrs;
    if(!XGetWindowAttributes(display_, window_, &attrs)) {
        Logger::Global().Log("Failed to get window attributes for framing", log_level::ERROR);
        return;
    }

    frame_ = XCreateSimpleWindow(
        display_, root, 
        attrs.x, attrs.y, attrs.width, attrs.height, 
        wm_constants::FRAME_BORDER_WIDTH,
        wm_constants::FRAME_BORDER_COLOR,
        wm_constants::FRAME_BG_COLOR);

    XSelectInput(display_, frame_, 
        SubstructureRedirectMask | ButtonPressMask);
                            
    XReparentWindow(display_, window_, frame_, 0, 0);
    XMapWindow(display_, frame_);
    is_framed_ = true;

    Logger::Global().Log("Create frame for window " + std::to_string(window_), log_level::DEBUG);
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
    if(is_framed_) {
        XMoveResizeWindow(display_, frame_, x, y, w, h);
        XMoveResizeWindow(display_, window_, 0, 0, w, h);
    } else {
        XMoveResizeWindow(display_, window_, x, y, w, h); // возможно такое и не выполняется никогда
    }

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
    if(is_framed_) {
        XUnmapWindow(display_, frame_);
        XReparentWindow(display_, window_, XRootWindow(display_, XDefaultScreen(display_)), 0, 0);
        XDestroyWindow(display_, frame_);
        frame_ = 0;
        is_framed_ = false;
        Logger::Global().Log("Destroyed frame for window " + std::to_string(window_), log_level::DEBUG);
    }

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