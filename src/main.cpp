#include "window_manager.hpp"
#include "logger.hpp"
#include "config.hpp"

int main() {
    try {
        auto wm = WindowManager::Create();
        wm->Run();
    } catch(const std::exception& e) {
        Logger::Global().Log(std::string(e.what()), log_level::ERROR);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}