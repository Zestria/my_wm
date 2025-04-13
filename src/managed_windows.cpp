/*
 * ManagedWindow Implementation
 *
 * This class represents a managed window in the window manager,
 * handling window decoration, focus, and basic operations.
 */

#include "managed_windows.hpp"
#include "logger.hpp"
#include "config.hpp"

/* 
 * Constructor - Initializes a new managed window
 * @param display: X11 display connection
 * @param window: X11 window ID to manage
 */
ManagedWindow::ManagedWindow(Display* display, Window window)
    : display_(display), window_(window), frame_(0), is_framed_(false) {}

/*
 * Creates a frame around the window
 * @param root: The root window used as parent for the frame
 */
void ManagedWindow::CreateFrame(Window root) {
    // Skip if already framed
    if(is_framed_) 
        return;

    // Get current window attributes
    XWindowAttributes attrs;
    if(!XGetWindowAttributes(display_, window_, &attrs)) {
        Logger::Global().Log("Failed to get window attributes for framing", log_level::ERROR);
        return;
    }

    // Create frame window with same dimensions as original
    frame_ = XCreateSimpleWindow(
        display_, root, 
        attrs.x, attrs.y, attrs.width, attrs.height, 
        wm_constants::BORDER_WIDTH,
        wm_constants::INACTIVE_BORDER,
        wm_constants::FRAME_BG_COLOR);
    
    // Reparent the original window into our frame
    XReparentWindow(display_, window_, frame_, 0, 0);
    is_framed_ = true;

    Logger::Global().Log("Create frame for window " + std::to_string(window_), log_level::DEBUG);
}

/*
 * Sets focus to this window and updates visual appearance
 */
void ManagedWindow::Focus() {
    if(!is_framed_) 
        return;
    
    is_focused_ = true;
    XSetInputFocus(display_, window_, RevertToPointerRoot, CurrentTime);
    XSetWindowBorder(display_, frame_, wm_constants::ACTIVE_BORDER);
    Logger::Global().Log("Focused window " + std::to_string(window_), log_level::INFO);
}

/*
 * Removes focus from this window and updates visual appearance
 */
void ManagedWindow::Unfocus() {
    if(!is_framed_) 
        return;

    is_focused_ = false;
    XSetWindowBorder(display_, frame_, wm_constants::INACTIVE_BORDER);
    
}

/*
 * Moves and resizes both the frame and client window
 * @param x,y: New position coordinates
 * @param w,h: New dimensions
 */
void ManagedWindow::MoveResize(int x, int y, int w, int h) {
    if(!is_framed_) 
        return;

    // Client window fills the frame (offset by border width)
    XMoveResizeWindow(display_, window_, 0, 0, w, h);
    // Position the frame itself
    XMoveResizeWindow(display_, frame_, x, y, w, h);
    Logger::Global().Log(
        "Moved window " + std::to_string(window_) + 
        " to [" + std::to_string(x) + "," + std::to_string(y) +
        "] size " + std::to_string(w) + "x" + std::to_string(h), 
        log_level::DEBUG);
}

/*
 * Makes the window visible
 */
void ManagedWindow::Open() {
    XMapWindow(display_, window_);
    if(is_framed_) {
        XMapWindow(display_, frame_);
        Logger::Global().Log("Mapped framed window: " + std::to_string(frame_), log_level::DEBUG);
    } else {
        Logger::Global().Log("Mapped unframed window: " + std::to_string(window_), log_level::DEBUG);
    }
}

/*
 * Destroys the window and its frame
 */
void ManagedWindow::Close() {
    if(is_framed_) {
        XDestroyWindow(display_, frame_);
        Logger::Global().Log("Destroyed framed window " + std::to_string(window_), log_level::DEBUG);
    } else {
        XDestroyWindow(display_, window_);
        Logger::Global().Log("Closed window: " + std::to_string(window_), log_level::INFO);
    }    
} 

/*
 * Sets event mask and button grabs for the frame
 * @param event_mask: X11 event mask to enable
 */
void ManagedWindow::SetEventMask(long event_mask) {
     // Select events we want to receive
    XSelectInput(display_, frame_, event_mask);

    // Grab left mouse button for window management
    XGrabButton(display_, Button1, 0, frame_, False, 
        ButtonPressMask, GrabModeAsync, GrabModeAsync, None, None);

    Logger::Global().Log("Set event mask " + std::to_string(event_mask) + 
                        " for window " + std::to_string(window_), 
        log_level::DEBUG
    );
}