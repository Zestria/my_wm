#pragma once

#include <vector>
#include <memory>
#include <fstream>
#include <X11/Xlib.h>

#include "managed_windows.hpp"

class WindowManager 
{
private:
    Display* display_;
    Window root_;
    std::vector<ManagedWindow> windows_;
    ManagedWindow focused_window_;
    
    explicit WindowManager(Display* display);
    
    void OnMapRequest(const XMapRequestEvent& e);
    void OnConfigureRequest(const XConfigureRequestEvent& e);
    void OnButtonPress(const XButtonEvent& e);
    void OnKeyPress(const XKeyEvent& e);
    void TileWindows();
    
public:
    static std::unique_ptr<WindowManager> Create();
    
    WindowManager(const WindowManager&) = delete;
    WindowManager& operator=(const WindowManager&) = delete;
    
    ~WindowManager();
    
    void Run();
};