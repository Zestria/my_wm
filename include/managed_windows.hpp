#pragma once

#include <X11/Xlib.h>

class ManagedWindow {
private:
    Display* display_;
    Window window_;
    bool is_focused_ = false;
public:
    ManagedWindow(Display* display, Window window);
    
    void Focus();
    void Unfocus();
    void Close();
    void Open();
    void MoveResize(int x, int y, int w, int h);
    void SetEventMask(long event_mask);

    Window GetX11Window() const { return window_; }
    bool isFocused() const { return is_focused_; }
};