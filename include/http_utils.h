#ifndef HTTP_UTILS_H
#define HTTP_UTILS_H

#include <string>
#include <curl/curl.h>

void cleanup_curl();

std::string http_get(const std::string& url);

#endif