/* 
 * WindowManager Implementation
 * 
 * This file implements an X11 window manager with basic tiling functionality
 * and keyboard shortcuts.
 */

#include <X11/keysym.h>
#include <algorithm>
#include <unistd.h>
#include <cstdlib>

#include "window_manager.hpp"
#include "managed_windows.hpp"
#include "logger.hpp"
#include "x11_utils.hpp"
#include "config.hpp"

/* 
 * Factory method to create WindowManager instance
 * Returns nullptr if X11 display connection fails
 */
std::unique_ptr<WindowManager> WindowManager::Create() {
    Display* display = XOpenDisplay(nullptr);
    if(!display) {
        Logger::Global().Log("Failed to open X display", log_level::ERROR);
        return nullptr;
    }
    
    return std::unique_ptr<WindowManager>(new WindowManager(display));
}

/*
 * WindowManager Constructor
 * Initializes X11 connection and sets up key bindings
 */
WindowManager::WindowManager(Display* display) :
    display_(display),
    root_(DefaultRootWindow(display_)),
    focused_index_(-1) 
{
    // Initialize logging
    Logger::Global().setLogFile(wm_constants::LOG_PATH);

    // Set up X11 error handler
    XSetErrorHandler(x11_utils::X11ErrorHandler::Handle);

    // Select events we want to receive on root window
    XSelectInput(display_, root_, 
        SubstructureRedirectMask | // For window management
        ButtonPressMask |          // Mouse clicks
        KeyPressMask               // Keyboard input
    );

    // Register global key bindings
    XGrabKey(display_, XKeysymToKeycode(display_, wm_constants::CLOSE_WM_KEY),
    wm_constants::MOD_KEY, root_, True, GrabModeAsync, GrabModeAsync);

    XGrabKey(display_, XKeysymToKeycode(display_, wm_constants::CLOSE_WIN_KEY),
    wm_constants::MOD_KEY, root_, True, GrabModeAsync, GrabModeAsync);

    XGrabKey(display_, XKeysymToKeycode(display_, wm_constants::OPEN_WIN_KEY),
    wm_constants::MOD_KEY, root_, True, GrabModeAsync, GrabModeAsync);

    XGrabKey(display_, XKeysymToKeycode(display_, wm_constants::OPEN_LAUNCH_KEY),
    wm_constants::MOD_KEY, root_, True, GrabModeAsync, GrabModeAsync);
    
    Logger::Global().Log("WindowManager initialized", log_level::INFO);
}

/*
 * WindowManager Destructor
 * Cleans up all windows and closes X11 connection
 */
WindowManager::~WindowManager() {
    if(display_) {
        for(auto window : windows_)
            window.Close();

        XCloseDisplay(display_);
        Logger::Global().Log("Display connection closed", log_level::INFO);
    }
}

/*
 * Main event loop
 * Processes X11 events and dispatches to appropriate handlers
 */
void WindowManager::Run() {
    XEvent e;
    while(true) {
        XNextEvent(display_, &e);
        try {
            switch(e.type) {
                case MapRequest: 
                    OnMapRequest(e.xmaprequest); 
                    break;
                case ConfigureRequest: 
                    OnConfigureRequest(e.xconfigurerequest); 
                    break;
                case ButtonPress: 
                    OnButtonPress(e.xbutton); 
                    break;
                case KeyPress: 
                    OnKeyPress(e.xkey); 
                    break;
            }
        } catch(const std::exception& e) {
            Logger::Global().Log(std::string(e.what()), log_level::ERROR);
        }
    }
}

/*
 * Handles window map requests (new windows)
 */
void WindowManager::OnMapRequest(const XMapRequestEvent& e) {
    Logger::Global().Log("MapRequest for window: " + std::to_string(e.window), log_level::DEBUG);

    // Get window attributes
    XWindowAttributes attrs;
    if(!XGetWindowAttributes(display_, e.window, &attrs)) {
        Logger::Global().Log("Failed to get window attributes", log_level::ERROR);
        return;
    }

    // Create ManagedWindow wrapper
    ManagedWindow window(display_, e.window);

    // Skip override-redirect windows (like tooltips)
    if(attrs.override_redirect) {
        window.Open();
        return;
    }

    // Add to window list and set up frame
    windows_.emplace_back(std::move(window));
    windows_.back().CreateFrame(root_);
    windows_.back().Open();
    windows_.back().SetEventMask(ButtonPressMask);

    // Update focus
    if(focused_index_ != -1) {
        windows_[focused_index_].Unfocus();
    }

    windows_.back().Focus();
    focused_index_ = (int)windows_.size()-1;
    
    // Rearrange windows
    TileWindows();
}

/*
 * Handles window configuration requests (resize/move)
 */
void WindowManager::OnConfigureRequest(const XConfigureRequestEvent& e) { // заменить XConfigureWindow
    XWindowChanges changes = {
        .x = e.x,
        .y = e.y,
        .width = e.width,
        .height = e.height,
        .border_width = e.border_width,
        .sibling = e.above,
        .stack_mode = e.detail
    };
    XConfigureWindow(display_, e.window, e.value_mask, &changes);
}

/*
 * Handles keyboard shortcuts
 */
void WindowManager::OnKeyPress(const XKeyEvent& e) {
    // Get pressed key symbol
    KeySym keysym = x11_utils::EventProcessor::GetKeysymFromEvent(e);

    // Check for modifier key (usually Super/Windows key)
    if(!(e.state & wm_constants::MOD_KEY))
        return;


    if(keysym == wm_constants::CLOSE_WM_KEY) {
        // Shutdown window manager
        Logger::Global().Log("Shutting down WindowManager", log_level::INFO);

        for(ManagedWindow win : windows_)
            win.Close();

        XCloseDisplay(display_);
        display_ = nullptr;

        exit(EXIT_SUCCESS);
    } 
    else if(keysym == wm_constants::CLOSE_WIN_KEY && focused_index_ >= 0) {
        // Close focused window
        if(focused_index_ != -1) {
            Logger::Global().Log("Closing window " + std::to_string(windows_[focused_index_].GetX11Window()), log_level::INFO);
            windows_[focused_index_].Close();
            windows_.erase(windows_.begin()+focused_index_);
            focused_index_ = -1;
            TileWindows();
        }
    }
    else if(keysym == wm_constants::OPEN_WIN_KEY) {
        // Launch terminal
        LaunchApplication(wm_constants::TERMINAL);
    }
    else if(keysym == wm_constants::OPEN_LAUNCH_KEY) {
        // Launch application launcher
        LaunchApplication(wm_constants::LAUNCHER);
    }
}

/*
 * Handles mouse clicks on windows
 */
void WindowManager::OnButtonPress(const XButtonEvent& e) {
    // Ignore clicks on root window
    if(e.window == root_) return; 

    Logger::Global().Log("Pressed on window : " + std::to_string(e.window), log_level::DEBUG);

    // Find which managed window was clicked
    auto it = std::find_if(
        windows_.begin(), windows_.end(), x11_utils::WindowFinder::MakeFinder(e.window)
    );

    if(it == windows_.end()) {
        Logger::Global().Log("Ignored click on non-managed window: " +
            std::to_string(e.window), log_level::DEBUG);
        return;
    }

    // Update focus
    if(focused_index_ != -1)
        windows_[focused_index_].Unfocus();

    focused_index_ = (int)(it - windows_.begin());
    windows_[focused_index_].Focus();
    
    Logger::Global().Log("Focused window: " + std::to_string(windows_[focused_index_].GetFrame()), log_level::INFO);
}

/*
 * Tiles all windows horizontally
 */
void WindowManager::TileWindows() {
    if(windows_.empty()) return;

    int width = DisplayWidth(display_, DefaultScreen(display_));
    int height = DisplayHeight(display_, DefaultScreen(display_));
    int tile_width = width / windows_.size();

    // Position each window
    for(size_t i = 0; i < windows_.size(); ++i)
        windows_[i].MoveResize(i*tile_width, 0, tile_width, height);
}

/*
 * Launches external applications
 */
void WindowManager::LaunchApplication(const std::string& command) {
    pid_t pid = fork();
    if(pid == 0) {
        // Child process - execute command
        setsid();
        execlp(command.c_str(), command.c_str(), nullptr);
        exit(EXIT_FAILURE);
    }
    else if(pid > 0) {
        Logger::Global().Log("Launched application: " + command, log_level::INFO);
    }
    else {
        Logger::Global().Log("Failed to launch: " + command, log_level::ERROR);
    }
}