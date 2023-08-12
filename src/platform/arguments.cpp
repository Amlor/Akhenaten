#include "arguments.h"

#include "core/application.h"
#include "io/log.h"

#include <filesystem>
#include <unordered_map>

#if defined(_WIN32) || defined(_WIN64)
#include <ShlObj.h>
#else // Linux
#include <unistd.h>
#include <pwd.h>
#endif

#define CURSOR_SCALE_ERROR_MESSAGE "Option --cursor-scale must be followed by a scale value of 1, 1.5 or 2"
#define DISPLAY_SCALE_ERROR_MESSAGE "Option --display-scale must be followed by a scale value between 0.5 and 5"
#define UNKNOWN_OPTION_ERROR_MESSAGE "Option %s not recognized"

namespace {

char const* const CFG_FILE_NAME = "ozymandias.cfg";

enum class argument_type {
    DATA_DIRECTORY,
    WINDOW_MODE,
    RENDERER,
    DISPLAY_SCALE_PERCENTAGE,
    CURSOR_SCALE_PERCENTAGE,
    WINDOW_WIDTH,
    WINDOW_HEIGHT,
};

const std::unordered_map<std::string, argument_type> argument_types{{"data_directory", argument_type::DATA_DIRECTORY},
                                                                    {"window_mode", argument_type::WINDOW_MODE},
                                                                    {"renderer", argument_type::RENDERER},
                                                                    {"display_scale_percentage",
                                                                     argument_type::DISPLAY_SCALE_PERCENTAGE},
                                                                    {"cursor_scale_percentage",
                                                                     argument_type::CURSOR_SCALE_PERCENTAGE},
                                                                    {"window_width", argument_type::WINDOW_WIDTH},
                                                                    {"window_height", argument_type::WINDOW_HEIGHT}};

void set_value(Arguments& arguments, argument_type type, std::string&& value) {
    switch (type) {
    case argument_type::DATA_DIRECTORY:
        arguments.set_data_directory(value);
        break;
    case argument_type::WINDOW_MODE:
        if (value == "1")
            arguments.set_window_mode();
        else
            arguments.set_fullscreen();
        break;
    case argument_type::RENDERER:
        arguments.set_renderer(value);
        break;
    case argument_type::DISPLAY_SCALE_PERCENTAGE:
        arguments.set_display_scale_percentage(std::stoi(value));
        break;
    case argument_type::CURSOR_SCALE_PERCENTAGE:
        arguments.set_cursor_scale_percentage(std::stoi(value));
        break;
    case argument_type::WINDOW_WIDTH: {
        auto v = arguments.get_window_size();
        v.w = std::stoi(value);
        arguments.set_window_size(v);
        break;
    }
    case argument_type::WINDOW_HEIGHT: {
        auto v = arguments.get_window_size();
        v.h = std::stoi(value);
        arguments.set_window_size(v);
        break;
    }
    };
}

/// Create formatted string
/// NOTE: (use C++20 as soon as it will be available to get rid of this)
template <typename... Args>
auto string_format(char const* format, Args... args) {
    const int string_size = std::snprintf(nullptr, 0, format, args...) + 1; // With space for '\0'

    if (string_size <= 0)
        app::terminate("Error during string formatting.");

    std::unique_ptr<char[]> buf(new char[string_size]);
    std::snprintf(buf.get(), string_size, format, args...);
    return std::string(buf.get(), buf.get() + string_size - 1); // Without '\0'
}

std::string get_configuration_path() {
#if defined(_WIN32) || defined(_WIN64)
    TCHAR app_data_path[MAX_PATH];

    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, app_data_path))) {
        return std::string(app_data_path) + '/' + CFG_FILE_NAME;
    }

    logs::error("Failed to retrieve AppData path.");
#else // Linux
    struct passwd* pw = getpwuid(getuid());
    std::string folder_path = std::string(pw->pw_dir) + "/.config/";
    if (std::filesystem::exists(folder_path))
        return folder_path + CFG_FILE_NAME;
#endif
    logs::warn("Home folder to keep configuration file was not found");

    return CFG_FILE_NAME;
}

} // namespace

static int parse_decimal_as_percentage(const char* str) {
    const char* start = str;
    char* end;
    long whole = SDL_strtol(start, &end, 10);
    int percentage = 100 * (int)whole;
    if (*end == ',' || *end == '.') {
        end++;
        start = end;
        long fraction = SDL_strtol(start, &end, 10);
        switch (end - start) {
        case 0:
            break;
        case 1:
            percentage += fraction * 10;
            break;
        case 2:
            percentage += fraction;
            break;
        default: {
            int fraction_digits = (int)(end - start);
            while (fraction_digits > 2) {
                fraction = fraction / 10;
                fraction_digits--;
            }
            percentage += fraction;
            break;
        }
        }
    }
    if (*end) {
        // still some characters left, print out warning
        logs::warn("Invalid decimal: %s", str);
        return -1;
    }
    return percentage;
}

Arguments::Arguments(int argc, char** argv)
  : data_directory_(std::filesystem::current_path().string()) {
    arguments::load(*this);
    parse_cli_(argc, argv);
}

char const* Arguments::usage() {
    return "Usage: ozymandias [ARGS] [DATA_DIR]\n"
           "\n"
           "ARGS may be:\n"
           "  --display-scale NUMBER\n"
           "          Scales the display by a factor of NUMBER. Number can be between 0.5 and 5\n"
           "  --cursor-scale NUMBER\n"
           "          Scales the mouse cursor by a factor of NUMBER. Number can be 1, 1.5 or 2\n"
           "  --render\n"
           "          use specific renderer\n"
           "  --size\n"
           "          window size. Example: 800x600\n"
           "  --window\n"
           "          enable window mode\n"
           "\n"
           "The last argument, if present, is interpreted as data directory of the Pharaoh installation";
}

bool Arguments::is_fullscreen() const {
    return !window_mode_;
}

void Arguments::set_fullscreen() {
    window_mode_ = false;
}

bool Arguments::is_window_mode() const {
    return window_mode_;
}

void Arguments::set_window_mode(bool flag) {
    window_mode_ = flag;
}

int Arguments::get_display_scale_percentage() const {
    return display_scale_percentage_;
}

void Arguments::set_display_scale_percentage(int value) {
    if (value < 50 || value > 500)
        app::terminate(DISPLAY_SCALE_ERROR_MESSAGE);

    display_scale_percentage_ = value;
}

int Arguments::get_cursor_scale_percentage() const {
    return cursor_scale_percentage_;
}

void Arguments::set_cursor_scale_percentage(int value) {
    if (value != 100 && value != 150 && value != 200)
        app::terminate(CURSOR_SCALE_ERROR_MESSAGE);

    cursor_scale_percentage_ = value;
}

std::string Arguments::get_renderer() const {
    return renderer_;
}

void Arguments::set_renderer(std::string value) {
    renderer_ = std::move(value);
}

std::string Arguments::get_data_directory() const {
    return data_directory_;
}

void Arguments::set_data_directory(std::string value) {
    data_directory_ = std::move(value);
}

display_size Arguments::get_window_size() const {
    return window_size_;
}

void Arguments::set_window_size(display_size value) {
    window_size_ = value;
}

void Arguments::parse_cli_(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        // ignore "-psn" arguments, this is needed to launch the app
        // from the Finder on macOS.
        // https://hg.libsdl.org/SDL/file/c005c49beaa9/test/testdropfile.c#l47
        if (SDL_strcmp(argv[i], "-psn") == 0)
            continue;
        else if (SDL_strcmp(argv[i], "--window") == 0)
            window_mode_ = true;
        else if (SDL_strcmp(argv[i], "--render") == 0) {
            if (i + 1 < argc) {
                renderer_ = argv[i + 1];
                ++i;
            } else
                app::terminate(DISPLAY_SCALE_ERROR_MESSAGE);
        } else if (SDL_strcmp(argv[i], "--display-scale") == 0) {
            if (i + 1 < argc) {
                int percentage = parse_decimal_as_percentage(argv[i + 1]);
                ++i;

                set_display_scale_percentage(percentage);
            } else
                app::terminate(DISPLAY_SCALE_ERROR_MESSAGE);
        } else if (SDL_strcmp(argv[i], "--size") == 0) {
            if (i + 1 < argc) {
                SDL_sscanf(argv[i + 1], "%dx%d", &window_size_.w, &window_size_.h);
                ++i;
            } else
                app::terminate(DISPLAY_SCALE_ERROR_MESSAGE);
        } else if (SDL_strcmp(argv[i], "--cursor-scale") == 0) {
            if (i + 1 < argc) {
                int percentage = parse_decimal_as_percentage(argv[i + 1]);
                ++i;

                set_cursor_scale_percentage(percentage);
            } else
                app::terminate(CURSOR_SCALE_ERROR_MESSAGE);
        } else if (SDL_strcmp(argv[i], "--help") == 0)
            app::terminate(usage());
        else if (SDL_strncmp(argv[i], "--", 2) == 0) {
            app::terminate(string_format(UNKNOWN_OPTION_ERROR_MESSAGE, argv[i]));
        } else {
            // TODO: ???? check that there are no other arguments after
            data_directory_ = argv[i];
        }
    }
}

namespace arguments {

void load(Arguments& arguments) {
    std::ifstream input(get_configuration_path(), std::ios::in);

    if (!input.is_open())
        logs::info("Configuration file was not found.");
    else {
        std::string line;
        while (std::getline(input, line)) {
            auto pos = line.find('=');
            if (pos == std::string::npos)
                continue;

            auto const key = line.substr(0, pos);
            auto const it = argument_types.find(key);
            if (it == argument_types.end()) {
                logs::warn("Unknown argument key: %s", key.c_str());
                continue;
            }

            set_value(arguments, it->second, line.substr(pos + 1));
        }
    }
}

void store(Arguments const& arguments) {
    std::ofstream output(get_configuration_path(), std::ios::trunc | std::ios::out);

    output << "data_directory" << '=' << arguments.get_data_directory() << '\n';
    output << "window_mode" << '=' << arguments.is_window_mode() << '\n';
    output << "renderer" << '=' << arguments.get_renderer() << '\n';
    output << "display_scale_percentage" << '=' << arguments.get_display_scale_percentage() << '\n';
    output << "cursor_scale_percentage" << '=' << arguments.get_cursor_scale_percentage() << '\n';
    output << "window_width" << '=' << arguments.get_window_size().w << '\n';
    output << "window_height" << '=' << arguments.get_window_size().h << '\n';
}

} // namespace arguments
