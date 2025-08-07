#pragma once

#include <chrono>
#include <format>
#include <iostream>
#include <mutex>
#include <string_view>

class Logger
{
public:
    enum class Level
    {
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };

    static void Log (Level level, std::string_view message)
    {
        std::lock_guard<std::mutex> lock (log_mutex);
        auto now = std::chrono::system_clock::now ();
        auto timestamp = std::format ("{:%Y-%m-%d %H:%M:%S}", now);
        std::cout << std::format ("{}[{}] {}: {}{}\n",
                GetLevelColor (level), // Start color
                timestamp, GetLevelString (level), message,
                "\033[0m" // Reset color at the end
        );
    }

    template<typename... Args>
    static void Log (Level level, std::format_string<Args...> fmt, Args &&...args)
    {
        Log (level, std::format (fmt, std::forward<Args> (args)...));
    }

private:
    static std::string_view GetLevelString (Level level)
    {
        switch (level)
        {
            case Level::DEBUG:
                return "DEBUG";
            case Level::INFO:
                return "INFO";
            case Level::WARNING:
                return "WARNING";
            case Level::ERROR:
                return "ERROR";
            default:
                return "UNKNOWN";
        }
    }

    static std::string_view GetLevelColor (Level level)
    {
        switch (level)
        {
            case Level::DEBUG:
                return "\033[35m"; // Magenta
            case Level::INFO:
                return "\033[32m"; // Green
            case Level::WARNING:
                return "\033[33m"; // Yellow
            case Level::ERROR:
                return "\033[31m"; // Red
            default:
                return "\033[0m"; // Reset
        }
    }

    static inline std::mutex log_mutex;
};
