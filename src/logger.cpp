#include <ctime>
#include <iomanip>

#include "logger.hpp"

Logger& Logger::Global() {
    static Logger instance;
    return instance;
}

void Logger::setLogFile(const std::string& path) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    logfile_.open(path, std::ios::app);
}

void Logger::Log(const std::string& message, const std::string& level) {
    std::lock_guard<std::mutex> lock(log_mutex_);

    if(!logfile_.is_open()) return;
    
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);

    logfile_ << std::put_time(&tm, "[%Y-%m-%d %H:%M:%S] ") 
             << "[" << level << "] "
             << message << std::endl;

    
}