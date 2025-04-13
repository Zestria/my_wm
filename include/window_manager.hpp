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
    Display* display_;          // Connection to X server
    Window root_;               // Root window reference
    
    // Window Management
    std::vector<ManagedWindow> windows_;  // Tracked windows
    int focused_index_;         // Index of focused window

    // Frame Management
    void FrameWindow(Window w);
    void UnframeWindow(Window w);
    
    // Private constructor (use Create() factory)
    explicit WindowManager(Display* display);
    
    // Event Handlers
    void OnMapRequest(const XMapRequestEvent& e);         // New window mapping
    void OnConfigureRequest(const XConfigureRequestEvent& e); // Resize/move
    void OnButtonPress(const XButtonEvent& e);            // Mouse clicks
    void OnKeyPress(const XKeyEvent& e);                  // Keyboard input

    void TileWindows(); // Arrange windows in tiled layout

    void LaunchApplication(const std::string& command); // Start programs
    
public:
    // Factory Method
    static std::unique_ptr<WindowManager> Create();
    
    // Prevent copying
    WindowManager(const WindowManager&) = delete;
    WindowManager& operator=(const WindowManager&) = delete;
    
    ~WindowManager();
    
    void Run(); // Event processing loop
};