#pragma once

#include <unordered_map>
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
    int focused_index_;

    void FrameWindow(Window w);
    void UnframeWindow(Window w);
    
    explicit WindowManager(Display* display);
    
    void OnMapRequest(const XMapRequestEvent& e);
    void OnConfigureRequest(const XConfigureRequestEvent& e);
    void OnButtonPress(const XButtonEvent& e);
    void OnKeyPress(const XKeyEvent& e);
    void TileWindows();

    void LaunchApplication(const std::string& command);
    
public:
    static std::unique_ptr<WindowManager> Create();
    
    WindowManager(const WindowManager&) = delete;
    WindowManager& operator=(const WindowManager&) = delete;
    
    ~WindowManager();
    
    void Run();
};