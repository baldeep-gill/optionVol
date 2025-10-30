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
    std::ofstream file(path + path, std::ios::trunc);
    if (!file.is_open()) {
        std::cerr << "Failed to write file: " << path << "\n";
        return;
    }

    file << data;
    file.close();
}

std::string generate_filename(const std::string& prefix, const std::string& ticker) {
    std::string safe_ticker = ticker;
    std::replace(safe_ticker.begin(), safe_ticker.end(), ':', '_');

    return fs::current_path().parent_path().string() + "/" + "cache/" + prefix + "_" + safe_ticker + ".json";
}