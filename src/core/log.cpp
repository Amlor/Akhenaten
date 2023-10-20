#include "core/log.h"

#include "platform/platform.h"
#include "core/application.h"
#include "widget/debug_console.h"

#include <algorithm>
#include <csignal>
#include <iostream>
#include <unordered_map>

#ifdef CPPTRACE_ENABLED
#include <cpptrace/cpptrace.hpp>

#include <iomanip>
#include <sstream>
#endif // CPPTRACE_ENABLED

#if defined(GAME_PLATFORM_WIN)
#include <Windows.h>
#elif defined(GAME_PLATFORM_ANDROID)
#include <android/log.h>
#endif

namespace {

pcstr logger_filename_ = "akhenaten-log.txt";
static std::fstream logger_file_stream_;

const std::unordered_map<std::string, SDL_LogPriority> PRIORITY_DICT = {
    {"verbose", SDL_LOG_PRIORITY_VERBOSE},
    {"debug", SDL_LOG_PRIORITY_DEBUG},
    {"info", SDL_LOG_PRIORITY_INFO},
    {"warn", SDL_LOG_PRIORITY_WARN},
    {"error", SDL_LOG_PRIORITY_ERROR},
    {"critical", SDL_LOG_PRIORITY_CRITICAL}
};

const std::unordered_map<SDL_LogPriority, char const*> PRIORITY_PREFIX = {
    {SDL_LOG_PRIORITY_VERBOSE, ""},
    {SDL_LOG_PRIORITY_DEBUG, "debug: "},
    {SDL_LOG_PRIORITY_INFO, ""},
    {SDL_LOG_PRIORITY_WARN, "warn: "},
    {SDL_LOG_PRIORITY_ERROR, "error: "},
    {SDL_LOG_PRIORITY_CRITICAL, "critical: "}
};

char const* get_prefix_of(SDL_LogPriority priority) {
    auto it = PRIORITY_PREFIX.find(priority);
    if (it != PRIORITY_PREFIX.end()) {
        return it->second;
    }

    app::terminate("Unknown SDL_LogPriority level");
}

SDL_LogPriority get_log_priority() {
    if (char const* priority_ptr = std::getenv("SDL_LOG_PRIORITY")) {
        auto priority_str = std::string(priority_ptr);
        std::transform(priority_str.begin(), priority_str.end(), priority_str.begin(), [](unsigned char c) {
            return std::tolower(c);
        });

        auto it = PRIORITY_DICT.find(priority_str);
        if (it != PRIORITY_DICT.end())
            return it->second;

        std::cerr << "Unknown SDL_LOG_PRIORITY value, VERBOSE will be used" << std::endl;
        return SDL_LOG_PRIORITY_VERBOSE;
    }

    return SDL_LOG_PRIORITY_INFO;
}

void sig_handler(int /* signal */) {
#ifdef CPPTRACE_ENABLED
    auto const trace = cpptrace::generate_trace();
    std::ostringstream output_stream;

    auto const frame_number_width = std::to_string(static_cast<int>(trace.size()) - 1).length();
    std::size_t counter = 0;

    output_stream << "Stack Trace:";
    for (auto const& frame: trace)
    {
        output_stream
            << "\n#"
            << std::setw(static_cast<int>(frame_number_width))
            << std::left
            << counter++
            << std::right
            << " "
            << std::hex
            << "0x"
            << std::setw(2 * sizeof(uintptr_t))
            << std::setfill('0')
            << frame.address
            << std::dec
            << std::setfill(' ')
            << " in "
            << frame.symbol
            << " at "
            << frame.filename
            << ":"
            << frame.line
            << (frame.col > 0 ? ":" + std::to_string(frame.col): "");
    }

    logs::critical(output_stream.str().c_str());
#endif // CPPTRACE_ENABLED
    exit(EXIT_FAILURE);
}

} // namespace

namespace logs {

void initialize() {
    signal(SIGSEGV, sig_handler);
    signal(SIGABRT, sig_handler);

    SDL_LogSetOutputFunction(Logger::write, nullptr);
    SDL_LogSetAllPriority(get_log_priority());
}

void switch_output(pcstr folder) {
    logger_file_stream_.close();

    bstring256 filename(folder, "/", logger_filename_);
    logger_file_stream_.open(filename, std::fstream::out | std::fstream::trunc);
}

} // namespace logs

Logger::Logger() {
    logger_file_stream_.open(logger_filename_, std::fstream::out | std::fstream::trunc);
}

Logger::~Logger() {
    logger_file_stream_.close();
}

void Logger::write(void* /* userdata */, int /* category */, SDL_LogPriority priority, char const* message) {
    static Logger logger;
    char const* const prefix = get_prefix_of(priority);

    Logger::write_to_output_(prefix, message);
    logger.write_to_file_(prefix, message);
}

void Logger::write_to_file_(char const* prefix, char const* message) {
    logger_file_stream_ << prefix << message << std::endl;

#if defined(GAME_PLATFORM_WIN)
    OutputDebugStringA(prefix);
    OutputDebugStringA(message);
    OutputDebugStringA("\n");
#elif defined(GAME_PLATFORM_ANDROID)
    __android_log_print(ANDROID_LOG_INFO, "ozy-and", "%s%s", prefix, message);
#endif
    debug_append_console_message(message);
}

void Logger::write_to_output_(char const* prefix, char const* message) {
    std::cout << prefix << message << std::endl;
}