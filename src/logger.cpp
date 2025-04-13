/*
* Logger Implementation
*
* Thread-safe logging utility with timestamping and file output.
* Implements singleton pattern for global access.
*/

#include <ctime>
#include <iomanip>

#include "logger.hpp"

/*
 * Singleton accessor
 * @return: Reference to the global Logger instance
 */
Logger& Logger::Global() {
    static Logger instance;
    return instance;
}

/*
 * Sets the output log file path
 * @param path: Filesystem path for log output
 */
void Logger::setLogFile(const std::string& path) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    logfile_.open(path, std::ios::app);
}

/*
 * Logs a message with specified severity level
 * @param message: The log message content
 * @param level: Severity level (DEBUG, INFO, ERROR)
 */
void Logger::Log(const std::string& message, const std::string& level) {
    std::lock_guard<std::mutex> lock(log_mutex_);

    if(!logfile_.is_open()) return;
    
    // Get current time
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);

    // Format: [timestamp] [LEVEL] message
    logfile_ << std::put_time(&tm, "[%Y-%m-%d %H:%M:%S] ") 
             << "[" << level << "] "
             << message << std::endl;

    
}