#include <X11/keysym.h>
#include <algorithm>
#include <unistd.h>
#include <cstdlib>

#include "window_manager.hpp"
#include "managed_windows.hpp"
#include "logger.hpp"
#include "x11_utils.hpp"
#include "config.hpp"

std::unique_ptr<WindowManager> WindowManager::Create() {
    Display* display = XOpenDisplay(nullptr);
    if(!display) {
        Logger::Global().Log("Failed to open X display", log_level::ERROR);
        return nullptr;
    }
    
    return std::unique_ptr<WindowManager>(new WindowManager(display));
}

WindowManager::WindowManager(Display* display) :
    display_(display),
    root_(DefaultRootWindow(display_)),
    focused_index_(-1) {
    
    Logger::Global().setLogFile(wm_constants::LOG_PATH);

    XSetErrorHandler(x11_utils::X11ErrorHandler::Handle);

    XSelectInput(display_, root_, 
        SubstructureRedirectMask |
        ButtonPressMask |
        KeyPressMask
    );

    XGrabKey(display_, XKeysymToKeycode(display_, wm_constants::CLOSE_WM_KEY),
    wm_constants::MOD_KEY, root_, True, GrabModeAsync, GrabModeAsync);

    XGrabKey(display_, XKeysymToKeycode(display_, wm_constants::CLOSE_WIN_KEY),
    wm_constants::MOD_KEY, root_, True, GrabModeAsync, GrabModeAsync);

    XGrabKey(display_, XKeysymToKeycode(display_, wm_constants::OPEN_WIN_KEY),
    wm_constants::MOD_KEY, root_, True, GrabModeAsync, GrabModeAsync);
    
    Logger::Global().Log("WindowManager initialized", log_level::INFO);
}

WindowManager::~WindowManager() {
    if(display_) {
        for(auto window : windows_)
            window.Close();

        XCloseDisplay(display_);
        Logger::Global().Log("Display connection closed", log_level::INFO);
    }
}

void WindowManager::Run() {
    XEvent e;
    while(true) {
        XNextEvent(display_, &e);
        try {
            switch(e.type) {
                case MapRequest: OnMapRequest(e.xmaprequest); break;
                case ConfigureRequest: OnConfigureRequest(e.xconfigurerequest); break;
                case ButtonPress: OnButtonPress(e.xbutton); break;
                case KeyPress: OnKeyPress(e.xkey); break;
            }
        } catch(const std::exception& e) {
            Logger::Global().Log(std::string(e.what()), log_level::ERROR);
        }
    }
}

void WindowManager::OnMapRequest(const XMapRequestEvent& e) {
    Logger::Global().Log("MapRequest for window: " + std::to_string(e.window), log_level::DEBUG);

    XWindowAttributes attrs;
    if(!XGetWindowAttributes(display_, e.window, &attrs)) {
        Logger::Global().Log("Failed to get window attributes", log_level::ERROR);
        return;
    }

    ManagedWindow window(display_, e.window);

    if(attrs.override_redirect) {
        window.Open();
        return;
    }

    windows_.emplace_back(std::move(window));
    windows_.back().CreateFrame(root_);
    windows_.back().Open();
    windows_.back().SetEventMask(ButtonPressMask);

    if(focused_index_ != -1) {
        windows_[focused_index_].Unfocus();
    }

    windows_.back().Focus();
    focused_index_ = (int)windows_.size()-1;
    
    TileWindows();
}

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

void WindowManager::OnKeyPress(const XKeyEvent& e) {
    KeySym keysym = x11_utils::EventProcessor::GetKeysymFromEvent(e);

    if(keysym == wm_constants::CLOSE_WM_KEY && (e.state & wm_constants::MOD_KEY)) {
        Logger::Global().Log("Shutting down WindowManager", log_level::INFO);

        for(ManagedWindow win : windows_)
            win.Close();

        XCloseDisplay(display_);
        display_ = nullptr;

        exit(EXIT_SUCCESS);
    } 
    else if(keysym == wm_constants::CLOSE_WIN_KEY && (e.state & wm_constants::MOD_KEY) && focused_index_ >= 0) {
        if(focused_index_ != -1) {
            Logger::Global().Log("Closing window " + std::to_string(windows_[focused_index_].GetX11Window()), log_level::INFO);
            windows_[focused_index_].Close();
            windows_.erase(windows_.begin()+focused_index_);
            focused_index_ = -1;
            TileWindows();
        }
    }
    else if(keysym == wm_constants::OPEN_WIN_KEY && (e.state & wm_constants::MOD_KEY)) {
        LaunchApplication(wm_constants::TERMINAL);
    }
}

void WindowManager::OnButtonPress(const XButtonEvent& e) {
    if(e.window == root_) return; 

    Logger::Global().Log("Pressed on window : " + std::to_string(e.window), log_level::DEBUG);

    auto it = std::find_if(
        windows_.begin(), windows_.end(), x11_utils::WindowFinder::MakeFinder(e.window)
    );

    if(it == windows_.end()) {
        Logger::Global().Log("Ignored click on non-managed window: " +
            std::to_string(e.window), log_level::DEBUG);
        return;
    }

    if(focused_index_ != -1)
        windows_[focused_index_].Unfocus();

    focused_index_ = (int)(it - windows_.begin());
    windows_[focused_index_].Focus();
    
    Logger::Global().Log("Focused window: " + std::to_string(windows_[focused_index_].GetFrame()), log_level::INFO);
}

void WindowManager::TileWindows() {
    if(windows_.empty()) return;

    int width = DisplayWidth(display_, DefaultScreen(display_));
    int height = DisplayHeight(display_, DefaultScreen(display_));
    int tile_width = width / windows_.size();

    for(size_t i = 0; i < windows_.size(); ++i)
        windows_[i].MoveResize(i*tile_width, 0, tile_width, height);
}

void WindowManager::LaunchApplication(const std::string& command) {
    pid_t pid = fork();
    if(pid == 0) {
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