#pragma once
#include <string>
#include <filesystem>
#include <optional>
#include <algorithm>
#include "thread_pool.h"

bool cache_exists(const std::string& path);

std::optional<std::string> read_cache(const std::string& path);

void write_cache(const std::string& path, const std::string& data, ThreadPool& pool);

std::string generate_filename(const std::string& prefix, const std::string& ticker);