#pragma once

#include <X11/Xlib.h>
#include <memory>

class ManagedWindow {
private:
    Display* display_;
    Window window_;
    Window frame_ = 0;
    bool is_focused_ = false;
    bool is_framed_ = false;
public:
    ManagedWindow(Display* display, Window window);
    
    void Focus();
    void Unfocus();
    void Close();
    void Open();
    void MoveResize(int x, int y, int w, int h);
    void SetEventMask(long event_mask);

    void CreateFrame(Window root);
    void DestroyFrame();
    bool HasFrame() const { return is_framed_; }
    Window GetFrame() const { return frame_; }

    Window GetX11Window() const { return window_; }
    bool isFocused() const { return is_focused_; }
};