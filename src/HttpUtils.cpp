#include "HttpUtils.h"

class CurlHandleWrapper {
    public:
        CURL* curl;
        CurlHandleWrapper() : curl(curl_easy_init()) {}
        ~CurlHandleWrapper() {
            if (curl) curl_easy_cleanup(curl);
        }
};

thread_local CurlHandleWrapper curlWrapper;

std::string HttpUtils::apiKey;

std::string HttpUtils::http_get(const std::string& url) {
    std::string final_url = url + "&apiKey=" + HttpUtils::apiKey;
    std::string response;

    CURL* curl = curlWrapper.curl;
    curl_easy_setopt(curl, CURLOPT_FRESH_CONNECT, 0L);
    curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 0L);
    curl_easy_setopt(curl, CURLOPT_URL, final_url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1000L);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 15L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) std::cerr << "CURL error (" << url << "): " << curl_easy_strerror(res) << "\n";

    if (res == CURLE_OK) {
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        if (response_code == 429) {
            std::cerr << "Rate limited - backing off\n";
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    return response;
}

class CurlGlobalInit {
    public:
        CurlGlobalInit() {
            curl_global_init(CURL_GLOBAL_DEFAULT);
        }

        ~CurlGlobalInit() {
            curl_global_cleanup();
        }
};

static CurlGlobalInit curlInitInstance;
static HttpUtils httpUtil;