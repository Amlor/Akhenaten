#include "dir.h"

#include "content/vfs.h"
#include "core/string.h"
#include "config/config.h"
#include "content/content.h"
#include "platform/platform.h"

#include <stdlib.h>
#include <string.h>
#include <filesystem>
#include <filesystem>
#include <map>

namespace fs = std::filesystem;

#define BASE_MAX_FILES 100

struct dir_data_t {
    dir_listing listing;
    int max_files;
    char* cased_filename;
};

dir_data_t g_dir_data;

namespace vfs {

struct path_uri {
    vfs::path original;
    vfs::path lowered;
};

std::map<size_t, path_uri> path_cache;

vfs::path content_internal_path(pcstr path) {
    vfs::path corrected_path = platform_file_manager_get_base_path();
    if (corrected_path.back() != '/') {
        corrected_path.append('/');
    }

    corrected_path.append(path);
    corrected_path.replace('\\', '/');
    return corrected_path;
}

void content_cache_real_file_paths(pcstr f) {
    vfs::path folder = content_internal_path(f);
    if (!fs::exists(folder.c_str()) || !fs::is_directory(folder.c_str())) {
        return;
    }

    auto add_cached_path = [] (pcstr entry) {
        path_uri uri;
        uri.original = entry;
        uri.original.replace('\\', '/');
        uri.lowered = uri.original;
        uri.lowered.tolower();
        size_t lhash = uri.lowered.hash();

        path_cache.insert({lhash, uri});
    };

    add_cached_path(folder);

    for (const auto &entry : fs::directory_iterator(folder.c_str())) {
        if (!fs::is_regular_file(entry.path())) {
            continue;
        }

        add_cached_path(entry.path().string().c_str());
    }
}

void content_cache_paths() {
    pcstr folders[] = {
        "",
        "AUDIO",
        "AUDIO/Ambient",
        "AUDIO/Music",
        "AUDIO/Voice",
        "AUDIO/Wavs",
        "data"
    };

    for (const auto &folder : folders) {
        content_cache_real_file_paths(folder);
    }
}

static void allocate_listing_files(int min, int max) {
    auto &data = g_dir_data;
    for (int i = min; i < max; i++) {
        data.listing.files[i] = (char *)malloc(MAX_FILE_NAME * sizeof(char));
        data.listing.files[i][0] = 0;
    }
}

static void clear_dir_listing() {
    auto &data = g_dir_data;
    data.listing.num_files = 0;
    if (data.max_files <= 0) {
        data.listing.files = (char **)malloc(BASE_MAX_FILES * sizeof(char *));
        allocate_listing_files(0, BASE_MAX_FILES);
        data.max_files = BASE_MAX_FILES;
    } else {
        for (int i = 0; i < data.max_files; i++) {
            data.listing.files[i][0] = 0;
        }
    }
}

static void expand_dir_listing() {
    auto &data = g_dir_data;
    int old_max_files = data.max_files;

    data.max_files = 2 * old_max_files;
    data.listing.files = (char **)realloc(data.listing.files, data.max_files * sizeof(char *));
    allocate_listing_files(old_max_files, data.max_files);
}

static int compare_lower(const void *va, const void *vb) {
    // arguments are pointers to char*
    return string_compare_case_insensitive(*(const char **)va, *(const char **)vb);
}

static int add_to_listing(const char *filename) {
    auto &data = g_dir_data;

    if (data.listing.num_files >= data.max_files)
        expand_dir_listing();

    strncpy(data.listing.files[data.listing.num_files], filename, MAX_FILE_NAME);
    data.listing.files[data.listing.num_files][MAX_FILE_NAME - 1] = 0;
    ++data.listing.num_files;
    return LIST_CONTINUE;
}

const dir_listing *dir_find_files_with_extension(const char *dir, const char *extension) {
    auto &data = g_dir_data;

    clear_dir_listing();
    platform_file_manager_list_directory_contents(dir, TYPE_FILE, extension, add_to_listing);
    qsort(data.listing.files, data.listing.num_files, sizeof(char *), compare_lower);
    return &data.listing;
}

const dir_listing *dir_find_all_subdirectories(pcstr dir) {
    auto &data = g_dir_data;

    clear_dir_listing();
    platform_file_manager_list_directory_contents(dir, TYPE_DIR, 0, add_to_listing);
    qsort(data.listing.files, data.listing.num_files, sizeof(char *), compare_lower);
    return &data.listing;
}


std::vector<path> dir_find_all_subdirectories(vfs::path dir, bool) {
    auto *listing = dir_find_all_subdirectories(dir.c_str());
    std::vector<path> folders;
    for (int i = 0; i < listing->num_files; i++) {
         folders.push_back(listing->files[i]);
    }
    return folders;
}

static int compare_case(const char *filename) {
    auto &data = g_dir_data;

    if (string_compare_case_insensitive(filename, data.cased_filename) == 0) {
        strcpy(data.cased_filename, filename);
        return LIST_MATCH;
    }
    return LIST_NO_MATCH;
}

static int correct_case(pcstr dir, char *filename, int type) {
    auto &data = g_dir_data;

    data.cased_filename = filename;
    return platform_file_manager_list_directory_contents(dir, type, 0, compare_case) == LIST_MATCH;
}

vfs::path content_path(pcstr path) {
    vfs::path uri_path = content_internal_path(path);
    vfs::path orig_path = uri_path;
    size_t lhash = uri_path.tolower().hash();

    auto it = path_cache.find(lhash);
    if (it != path_cache.end()) {
        const bool is_equal = strcmp(it->second.lowered.c_str(), uri_path.c_str()) == 0;
        if (is_equal) {
            return it->second.original;
        }
    }

    return orig_path;
}

vfs::path content_file(const char *filepath) {
#ifndef GAME_PLATFORM_ANDROID
    vfs::path corrected_filename = content_path(filepath);
    bool exists = std::filesystem::exists(corrected_filename.c_str());
    if (exists) {
        return corrected_filename;
    }

    return vfs::path();
#else
    vfs::path corrected_filename = content_path(filepath);
    // works with native fs
    if (!corrected_filename.empty() && corrected_filename.data()[0] == '/') {
        return corrected_filename;
    }
    // works with content resolver
    return filepath;
#endif
}

const dir_listing *dir_append_files_with_extension(const char *dir, const char *extension) {
    auto &data = g_dir_data;

    platform_file_manager_list_directory_contents(dir, TYPE_FILE, extension, add_to_listing);
    qsort(data.listing.files, data.listing.num_files, sizeof(char *), compare_lower);
    return &data.listing;
}

} // vfs
