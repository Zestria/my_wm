#pragma once
#include <string>
#include <fstream>
#include <mutex>

class Logger {
private:
    std::ofstream logfile_; // Output file stream
    std::mutex log_mutex_;  // Thread safety

    // Singleton enforcement
    Logger() = default;
public:
    // Singleton Access
    static Logger& Global();

    // Configuration
    void setLogFile(const std::string& path);

    // Logging Operation
    void Log(const std::string& message, const std::string& level);
};