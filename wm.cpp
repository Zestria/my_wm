#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/X.h>
#include <X11/keysym.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <cerrno>
#include <vector>
#include <algorithm>

constexpr const char* LOG_PATH = "/tmp/mywm.log";

std::ofstream logfile;

void log_message(const std::string& message) {
    logfile << message << std::endl;
    std::cout << message << std::endl;
}

KeyCode enter_key, q_key;

void setup_hotkeys(Display *display, Window root) {

    enter_key = XKeysymToKeycode(display, XK_Return);
    q_key = XKeysymToKeycode(display, XK_q);

    if (enter_key == 0 || q_key == 0) {
        log_message("ERROR: Failed to bind hotkeys");
        return;
    }

    XUngrabKey(display, AnyKey, AnyModifier, root);

    if( XGrabKey(display, enter_key, Mod4Mask, root, True, GrabModeAsync, GrabModeAsync) != 0 ) {
        log_message("ERROR: Failed to grab Win+Enter");
    }
    if( XGrabKey(display, q_key, Mod4Mask, root, True, GrabModeAsync, GrabModeAsync) != 0) {
        log_message("ERROR: Failed to grab Win+Q");
    }
}

void setup_mouse(Display *display, Window root) {
    XUngrabButton(display, AnyButton, AnyModifier, root);

    if(XGrabButton(display, Button1, Mod4Mask, root, True,
                ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                GrabModeAsync, GrabModeAsync, None, None) != 0) 
    {
        log_message("Error: Failed to grab mouse button");
    } else {
        log_message("Mouse button grabbed succesfully");
    }
}

Window focused_window = None;

std::vector<Window> managed_windows;
// реализую пока только Split V
void tile_windows(Display* display, Window root)  {
    if(managed_windows.empty()) return;

    XWindowAttributes root_attrs;
    XGetWindowAttributes(display, root, &root_attrs);
    const int width = root_attrs.width;
    const int height = root_attrs.height;

    const int win_width = width / std::max( (int)managed_windows.size(), 1 );

    for(size_t i = 0; i < managed_windows.size(); ++i) {
        XMoveResizeWindow(display, managed_windows[i], i * win_width, 0, win_width, height);
    }
}

void focus_window(Display *display, Window window) {
    if(window == None || window == focused_window) return;

    XWindowAttributes attrs;
    if(!XGetWindowAttributes(display, window, &attrs)) {
        log_message("Focus failed: bad window " + std::to_string(window));
    }

    if(XSetInputFocus(display, window, RevertToPointerRoot, CurrentTime) == BadWindow) {
        log_message("Failed to set focus to window " + std::to_string(window));
        return;
    }
    XSetWindowBorder(display, window, WhitePixel(display, DefaultScreen(display)));

    if(focused_window != None && focused_window != window) {
        if(XGetWindowAttributes(display, focused_window, &attrs)) {
            XSetWindowBorder(display, focused_window, 
                BlackPixel(display, DefaultScreen(display)));
        }
    }

    focused_window = window;
    log_message("Focused window: "+ std::to_string(window));
}

void safe_destroy_window(Display *display, Window window, Window root) {
    if(window == None) {
        log_message("Attempt to destroy None window");
        return;
    }
    
    XWindowAttributes attrs;
    if( !XGetWindowAttributes(display, window, &attrs) ) {
        log_message("Remove failed: window " + std::to_string(window) + " doesn't exist");
        return;
    }

    std::erase(managed_windows, window);

    if(focused_window == window) {
        focused_window = None;
        XSetInputFocus(display, root, RevertToPointerRoot, CurrentTime);
    }

    XDestroyWindow(display, window);
    XSync(display, False);

    tile_windows(display, root);
    log_message("Window removed: " + std::to_string(window));
}


int main() {

    logfile.open(LOG_PATH, std::ios::app);
    if(!logfile.is_open()) {
        std::cerr << "Failed to open log file!" << std::endl;
        return 1;
    }

    Display* display = XOpenDisplay(nullptr);
    if(!display) {
        log_message("Cannot open X display");
        logfile.close();
        return 1;
    }

    log_message("WM starting...");

    Window root = DefaultRootWindow(display);

    auto old_handler = XSetErrorHandler([](Display*, XErrorEvent* ev) {
        char buff[256];
        XGetErrorText(ev->display, ev->error_code, buff, sizeof(buff));
        log_message("X11 Error: " + std::string(buff));
        return 0;
    });

    XSelectInput(display, root, 
        SubstructureRedirectMask | 
        SubstructureNotifyMask | 
        StructureNotifyMask |
        KeyPressMask | 
        ButtonPressMask
    );

    setup_hotkeys(display, root);

    XEvent event;
    while(true) {
        XNextEvent(display, &event);

        try {
            switch(event.type) {
                case MapRequest: {
                    Window child = event.xmaprequest.window;

                    XWindowAttributes attrs;
                    XGetWindowAttributes(display, child, &attrs);
                    if(attrs.override_redirect) {
                        log_message("Ignoring override_redirect window");
                        XMapWindow(display, child);
                        break;
                    }

                    XSelectInput(display, child, 
                        StructureNotifyMask |
                        ButtonPressMask);
                    if( XMapWindow(display, child) == BadAccess) {
                        log_message("Failed to map window: BadAccess");
                    }

                    if(std::find(managed_windows.begin(), managed_windows.end(), child) == managed_windows.end()) {
                        managed_windows.push_back(child);
                        tile_windows(display, root);
                    }

                    XSetWindowBorderWidth(display, child, 2);
                    if(focused_window == None) {
                        focus_window(display, child);
                    } else {
                        XSetWindowBorder(display, child, BlackPixel(display, DefaultScreen(display)));
                    }

                    break;
                }
                case ConfigureRequest: {
                    XConfigureRequestEvent conf = event.xconfigurerequest;
                    XWindowChanges changes = {
                        .x = conf.x,
                        .y = conf.y,
                        .width = conf.width,
                        .height = conf.height,
                        .border_width = conf.border_width,
                        .sibling = conf.above,
                        .stack_mode = conf.detail
                    };
                    XConfigureWindow(display, conf.window, conf.value_mask, &changes);
                    break;
                }
                case CreateNotify: {
                    log_message("Window created: " + std::to_string(event.xcreatewindow.window));
                    break;
                }
                case DestroyNotify: {
                    //Window dead_window = event.xdestroywindow.window;
                    //remove_window(display, dead_window, root);

                    break;
                }
                case KeyPress: {
                    XKeyEvent key_event = event.xkey;
    
                    if(key_event.keycode == enter_key && (key_event.state & Mod4Mask)) {
                        system("alacritty &");
                    } else if(key_event.keycode == q_key && (key_event.state & Mod4Mask)) {
                        if(focused_window != None) {
                            log_message("Closing window: " + std::to_string(focused_window));
                            //XDestroyWindow(display, focused_window);
                            safe_destroy_window(display, focused_window, root);
                        }
                    }
                    break;
                }
                case ButtonPress: {
                    Window target = event.xbutton.subwindow ? event.xbutton.subwindow : event.xbutton.window;

                    log_message("ButtonPress on: windows=" + std::to_string(event.xbutton.window) + 
                                "subwindow=" + std::to_string(event.xbutton.subwindow));

                    if( target != None && 
                        target != root && 
                        std::find(managed_windows.begin(), managed_windows.end(), target) != managed_windows.end() ) 
                    {
                        focus_window(display, target);
                    }
                    break;
                }
                case UnmapNotify: {
                    Window unmapped = event.xunmap.window;
                    if(event.xunmap.send_event) break; // Игнорируем события не от сервера
                    //remove_window(display, unmapped, root);
                }
                
            }
        } catch (const std::exception& e) {
            log_message("ERROR in event loop: " + std::string(e.what()));
            continue;
        }        
    }

    XSetErrorHandler(old_handler);
    XCloseDisplay(display);
    logfile.close();

    return 0;
}