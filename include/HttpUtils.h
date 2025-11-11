#pragma once
#include <string>
#include <thread>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <curl/curl.h>

class HttpUtils {
    public:
        explicit HttpUtils();
        static std::string http_get(const std::string& url);

    private:
        static size_t write_data(void* buffer, size_t size, size_t nmeb, std::string* output);
        static std::string apiKey;
};

inline HttpUtils::HttpUtils() {
    const char* key = std::getenv("POLYGON_API_KEY");
    if (!key) throw std::invalid_argument("No API key found in environment.");
    apiKey = key;
}

inline size_t HttpUtils::write_data(void* buffer, size_t size, size_t nmemb, std::string* userdata) {
    size_t total_size = size * nmemb;
    userdata->append((char*)buffer, total_size);
    return total_size;
}