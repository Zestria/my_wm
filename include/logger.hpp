#pragma once
#include <string>
#include <fstream>
#include <mutex>

class Logger {
private:
    std::ofstream logfile_;
    std::mutex log_mutex_;
    Logger() = default;
public:
    static Logger& Global();

    void setLogFile(const std::string& path);
    void Log(const std::string& message, const std::string& level);
};