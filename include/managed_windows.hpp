#pragma once

#include <X11/Xlib.h>
#include <memory>

class ManagedWindow {
private:
    // X11 Connection and Window References
    Display* display_;   // X11 display connection
    Window window_;      // Client window ID
    Window frame_ = 0;   // Frame window ID
    
    bool is_focused_ = false;  // Focus state
    bool is_framed_ = false;   // Frame existence
public:
    ManagedWindow(Display* display, Window window);
    
    // Window Operations
    void Focus();
    void Unfocus();
    void Close();
    void Open();
    void MoveResize(int x, int y, int w, int h);

    // Event Handling
    void SetEventMask(long event_mask);

    // Frame Management
    void CreateFrame(Window root);
    void DestroyFrame();

    // Accessors
    bool HasFrame() const { return is_framed_; }
    Window GetFrame() const { return frame_; }
    Window GetX11Window() const { return window_; }
    bool isFocused() const { return is_focused_; }
};