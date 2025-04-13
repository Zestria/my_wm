#include "managed_windows.hpp"
#include "logger.hpp"
#include "config.hpp"

ManagedWindow::ManagedWindow(Display* display, Window window)
    : display_(display), window_(window), frame_(0), is_framed_(false) {}

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
        wm_constants::BORDER_WIDTH,
        wm_constants::INACTIVE_BORDER,
        wm_constants::FRAME_BG_COLOR);
                            
    XReparentWindow(display_, window_, frame_, 0, 0);
    is_framed_ = true;

    Logger::Global().Log("Create frame for window " + std::to_string(window_), log_level::DEBUG);
}

void ManagedWindow::Focus() {
    if(!is_framed_) 
        return;
    
    is_focused_ = true;
    XSetInputFocus(display_, window_, RevertToPointerRoot, CurrentTime);
    XSetWindowBorder(display_, frame_, wm_constants::ACTIVE_BORDER);
    Logger::Global().Log("Focused window " + std::to_string(window_), log_level::INFO);
}

void ManagedWindow::Unfocus() {
    if(!is_framed_) 
        return;

    is_focused_ = false;
    XSetWindowBorder(display_, frame_, wm_constants::INACTIVE_BORDER);
    
}

void ManagedWindow::MoveResize(int x, int y, int w, int h) {
    if(!is_framed_) 
        return;

    XMoveResizeWindow(display_, window_, 0, 0, w, h);
    XMoveResizeWindow(display_, frame_, x, y, w, h);
    Logger::Global().Log(
        "Moved window " + std::to_string(window_) + 
        " to [" + std::to_string(x) + "," + std::to_string(y) +
        "] size " + std::to_string(w) + "x" + std::to_string(h), 
        log_level::DEBUG);
}

void ManagedWindow::Open() {
    XMapWindow(display_, window_);
    if(is_framed_) {
        XMapWindow(display_, frame_);
        Logger::Global().Log("Mapped framed window: " + std::to_string(frame_), log_level::DEBUG);
    } else {
        Logger::Global().Log("Mapped unframed window: " + std::to_string(window_), log_level::DEBUG);
    }
}

void ManagedWindow::Close() {
    if(is_framed_) {
        XDestroyWindow(display_, frame_);
        Logger::Global().Log("Destroyed framed window " + std::to_string(window_), log_level::DEBUG);
    } else {
        XDestroyWindow(display_, window_);
        Logger::Global().Log("Closed window: " + std::to_string(window_), log_level::INFO);
    }    
} 

void ManagedWindow::SetEventMask(long event_mask) {
    XSelectInput(display_, frame_, event_mask);

    XGrabButton(display_, Button1, 0, frame_, False, 
        ButtonPressMask, GrabModeAsync, GrabModeAsync, None, None);

    Logger::Global().Log("Set event mask " + std::to_string(event_mask) + 
                        " for window " + std::to_string(window_), 
        log_level::DEBUG
    );
}