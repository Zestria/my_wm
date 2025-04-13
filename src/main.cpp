/*
 * Main Application Entry Point
 * 
 * This file contains the main execution flow for the window manager.
 * It follows the RAII pattern for resource management and includes
 * basic exception handling.
 */

#include "window_manager.hpp"
#include "logger.hpp"
#include "config.hpp"

int main() {
    try {
        // Create WindowManager instance using factory method
        auto wm = WindowManager::Create();
        // Enter main event loop
        wm->Run();
    } catch(const std::exception& e) {
        // Catch and log any unhandled exceptions
        Logger::Global().Log(std::string(e.what()), log_level::ERROR);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}