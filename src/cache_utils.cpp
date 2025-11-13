#include "cache_utils.h"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

bool cache_exists(const std::string& path) {
    return fs::exists(path);
}

std::optional<std::string> read_cache(const std::string& path) {
    if (!fs::exists(path)) return std::nullopt;

    std::ifstream file(path);
    if (!file.is_open()) return std::nullopt;

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    return content;
}

void write_cache(const std::string& path, const std::string& data) {
    std::string cache_path = fs::current_path().parent_path().string() + "/cache";
    if (!fs::exists(cache_path)) {
        std::cout << "Creating path: " << cache_path << "\n";
        fs::create_directory(fs::current_path().parent_path().string() + "/cache");
    }

    std::ofstream file(path, std::ios::trunc);
    if (!file.is_open()) {
        std::cerr << "Failed to write file: " << path << "\n";
        return;
    }

    file << data;
    file.close();
}

std::string generate_filename(const std::string& prefix, const std::string& name) {
    std::string safe_name = name;
    std::replace(safe_name.begin(), safe_name.end(), ':', '_');

    return fs::current_path().parent_path().string() + "/" + "cache/" + prefix + "_" + name + ".json";
}