/**
 * @file logger.hpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-01-05
 * @copyright Copyright (c) 2024
 * @brief Logger class.
 */

#ifndef UTILS_LOGGER_H_
#define UTILS_LOGGER_H_

#include <string>
#include <vector>

// Helper macro: Converts an argument to a string
#define TO_STRING_DETAIL(x) #x

// Main macro: Uses the helper macro to convert an argument to a string
#define TO_STRING(x) TO_STRING_DETAIL(x)

// LOCATION macro: Returns the current file name, line number, and function name as a string
#define LOCATION (std::string(__FILE__) + ":" + std::to_string(__LINE__) + " (" + std::string(__func__) + ")").c_str()

// PRETTY_LOCATION macro: Returns the current file name, line number, and pretty function name as a string
#define PRETTY_LOCATION (std::string(__FILE__) + ":" + std::to_string(__LINE__) + " (" + std::string(__PRETTY_FUNCTION__) + ")").c_str()

namespace utils {

enum class LogLevel
{
    /**
     * TRACE: Very detailed logs, which may include high-volume information such as
     * loop iterations, data dumps, etc. Use this level for development and debugging
     * purposes to trace the program flow in detail.
     */
    TRACE,

    /**
     * DEBUG: Detailed information on the flow through the system. This is used for
     * interactive investigation and debugging to get more granular information.
     */
    DEBUG,

    /**
     * INFO: Informational messages that highlight the progress of the application
     * at a high level. This level is for general runtime events (startup/shutdown).
     */
    INFO,

    /**
     * WARN: Potentially harmful situations that indicate potential problems.
     * These messages are for issues that are recoverable or are not yet errors.
     */
    WARN,

    /**
     * ERROR: Error events that might still allow the application to continue running.
     * These messages are for serious issues that require attention.
     */
    ERROR,

    /**
     * FATAL: Very severe error events that will presumably lead the application to abort.
     * These messages indicate critical problems and often precede application termination.
     */
    FATAL
};

constexpr int     kMsgMaxLength  = 70;
const std::string kLogLevelTrace = "[TRACE]";
const std::string kLogLevelDebug = "[DEBUG]";
const std::string kLogLevelInfo  = "[INFO]";
const std::string kLogLevelWarn  = "[WARN]";
const std::string kLogLevelError = "[ERROR]";
const std::string kLogLevelFatal = "[FATAL]";
const std::string kDash          = "---------------------------------------------------------------------";

struct LogFormat {
    std::string log_level;
    std::string time_stamp;
    std::string func_name;
    std::string message;

    /**
     * @brief Default constructor for LogFormat class.
     *
     * Initializes the LogFormat object with empty strings for its member variables:
     * log_level, time_stamp, func_name, message, and detail.
     */
    LogFormat();

    /**
     * @brief Formats log information into a single string.
     *
     * Formats log information (log_level, time_stamp, func_name, message, and detail)
     * into a single string using the specified delimiter 'del'.
     *
     * @param del The delimiter used to separate log information in the output string.
     * @return A string containing formatted log information joined by the delimiter.
     */
    std::string Format(const std::string del = ",");
};

/**
 * @brief A simple logger for recording experiment logs.
 *
 * The `Logger` class provides a basic logging mechanism for recording experiment logs.
 */
class Logger {
public:
    Logger(){};

    /**
     * USEAGE:
     * utils::Logger::TraceLog(LOCATION, "This is TRACE log.", debug);
     */
    static void TraceLog(const std::string &location, const std::string &message, const bool debug);

    /**
     * USEAGE:
     * utils::Logger::DebugLog(LOCATION, "This is DEBUG log.", debug);
     */
    static void DebugLog(const std::string &location, const std::string &message, const bool debug);

    /**
     * USEAGE:
     * utils::Logger::InfoLog(LOCATION, "This is INFO log.");
     */
    static void InfoLog(const std::string &location, const std::string &message);

    /**
     * USEAGE:
     * utils::Logger::WarnLog(LOCATION, "This is WARN log.");
     */
    static void WarnLog(const std::string &location, const std::string &message);

    /**
     * USEAGE:
     * utils::Logger::ErrorLog(LOCATION, "This is ERROR log.");
     */
    static void ErrorLog(const std::string &location, const std::string &message);

    /**
     * USEAGE:
     * utils::Logger::FatalLog(LOCATION, "This is FATAL log.");
     */
    static void FatalLog(const std::string &location, const std::string &message);

    /**
     * @brief Save the logs to a file.
     *
     * This method saves the log entries stored in the log list to a specified file.
     * It also prints each log entry to the console. After saving, the log list is cleared.
     *
     * @param file_path The file path where the logs will be saved.
     * @param is_date_time A flag to indicate whether to include the date and time in the file name.
     */
    static void SaveLogsToFile(const std::string &file_path, const bool is_date_time = true);

    /**
     * @brief Returns a string with the specified message centered and surrounded by a separator.
     *
     * Ensures that the total length of the output string is exactly 'width' characters.
     * If the width is less than the length of the message, returns the message without separators.
     *
     * @param message The message to be centered and surrounded by the separator.
     * @param separator The character used to surround the message.
     * @param width The total width of the output string.
     * @return A string with the message centered and surrounded by the separator.
     */
    static std::string StrWithSep(const std::string &message, char separator = '-', int width = kMsgMaxLength);

private:
    static LogFormat                log_format_;
    static std::vector<std::string> log_list_; /**< A list to store log entries as strings. */
    static void                     SetLogFormat(const std::string &log_level, const std::string &func_name, const std::string &message);
};

}    // namespace utils

#endif    // UTILS_LOGGER_H_
