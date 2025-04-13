# MyWM - Custom X11 Window Manager

## Overview

MyWM is a lightweight, tiling window manager for the X Window System written in modern C++ (C++20). It provides basic window management functionality with keyboard-driven control and simple window decorations.

## Keyboard Controls

- Mod (Super/Windows key) + Enter: Launch terminal
- Mod + D: Launch application launcher (dmenu)
- Mod + Q: Close focused window
- Mod + F4: Exit window manager

## Build Requirements
- C++20
- Xlib (libx11-dev)
- CMake (>= 3.10)

## Building and Installation

```bash
# Clone repo
git clone https://github.com/Zestria/my_wm.git
cd my_wm

# Build
mkdir build && cd build
cmake ..
make

// Run
startx ./my_wm
```

## Future Improvements
- More layout modes
- Workspaces
- Fullscreen mode
