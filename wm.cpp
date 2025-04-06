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
constexpr int MAX_KEYCODE_TRIES = 3;

std::ofstream logfile;

void log_message(const std::string& message) {
    logfile << message << std::endl;
    std::cout << message << std::endl;
}

KeyCode get_keycode_with_fallback(Display *display, const KeySym *keysyms, size_t n) {
    for(size_t i = 0; i < n; i++) {
        KeyCode kc = XKeysymToKeycode(display, keysyms[i]);
        if(kc!=0) return kc;
    }
    return 0;
}

KeyCode win_key, enter_key;

void setup_hotkeys(Display *display, Window root) {

    const KeySym SUPER_KEYS[] = {XK_Super_L, XK_Meta_L, XK_Hyper_L};
    win_key = get_keycode_with_fallback(display, SUPER_KEYS, sizeof(SUPER_KEYS)/sizeof(SUPER_KEYS[0]));

    win_key = XKeysymToKeycode(display, XK_Super_L);
    enter_key = XKeysymToKeycode(display, XK_Return);

    if (enter_key == 0 || win_key == 0) {
        log_message("ERROR: Failed to bind hotkeys\n");
        return;
    }

    if( XGrabKey(display, enter_key, Mod4Mask, root, True, GrabModeAsync, GrabModeAsync) != 0 ) {
        log_message("ERROR: Failed to grab Win+Enter");
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

    const int win_width = width / managed_windows.size();
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

    XSetInputFocus(display, window, RevertToPointerRoot, CurrentTime);
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

void remove_window(Display* display, Window window, Window root) {

    XWindowAttributes attrs;
    if(window != None && !XGetWindowAttributes(display, window, &attrs)) {
        log_message("Remove failed: bad window" + std::to_string(window));
        return;
    }

    managed_windows.erase(
        std::remove(managed_windows.begin(), managed_windows.end(), window),
        managed_windows.end()
    );

    if(focused_window == window) {
        focused_window = None;
        XSetInputFocus(display, root, RevertToPointerRoot, CurrentTime);
    }

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
        ButtonPressMask);

    setup_hotkeys(display, root);

    

    XEvent event;
    while(true) {
        XNextEvent(display, &event);

        try {
            switch(event.type) {
                case MapRequest: {
                    Window child = event.xmaprequest.window;
                    XMapWindow(display, child);

                    if(std::find(managed_windows.begin(), managed_windows.end(), child) == managed_windows.end()) {
                        managed_windows.push_back(child);
                        tile_windows(display, root);
                    }

                    XSetWindowBorderWidth(display, child, 2);
                    if(focused_window == None) {
                        focus_window(display, child);
                    } else {
                        XSetWindowBorder(display, child, BlackPixel(display, 0));
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
                    Window dead_window = event.xdestroywindow.window;
                    remove_window(display, dead_window, root);
                    break;
                }
                case KeyPress: {
                    XKeyEvent key_event = event.xkey;
    
                    if(key_event.keycode == enter_key && (key_event.state & Mod4Mask)) {
                        system("alacritty &");
                    }
                    break;
                }
                case ButtonPress: {
                    Window clicked_window = event.xbutton.window;
                    if(clicked_window != root) {
                        focus_window(display, clicked_window);
                    }
                    break;
                }
                case UnmapNotify: {
                    Window unmapped = event.xunmap.window;
                    if(event.xunmap.send_event) break; // Игнорируем события не от сервера
                    remove_window(display, unmapped, root);
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