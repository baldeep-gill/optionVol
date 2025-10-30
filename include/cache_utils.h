#pragma once
#include <string>
#include <filesystem>
#include <optional>
#include <algorithm>

bool cache_exists(const std::string& path);

std::optional<std::string> read_cache(const std::string& path);

void write_cache(const std::string& path, const std::string& data);

std::string generate_filename(const std::string& prefix, const std::string& ticker);