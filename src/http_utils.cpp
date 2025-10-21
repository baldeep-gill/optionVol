#include "http_utils.h"
#include <iostream>

size_t write_data(void* buffer, size_t size, size_t nmemb, std::string* output) {
    size_t total_size = size * nmemb;
    output->append((char*)buffer, total_size);
    return total_size;
}

void init_curl() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

void cleanup_curl() {
    curl_global_cleanup();
}

std::string http_get(const std::string& url) {
    CURL* curl = curl_easy_init();
    std::string response;

    if (!curl) {
        std::cerr << "Failed to initialise for: " << url << "\n";
        return "";
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_FRESH_CONNECT, 0L);
    curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 0L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) std::cerr << "CURL error (" << url << "): " << curl_easy_strerror(res) << "\n";

    curl_easy_cleanup(curl);
    return response;
}