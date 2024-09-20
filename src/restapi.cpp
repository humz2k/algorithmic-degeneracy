#include "restapi.hpp"

namespace deg {
namespace rest {

void init() { curl_global_init(CURL_GLOBAL_ALL); }

void deinit() { curl_global_cleanup(); }

size_t write_data(void* buffer, size_t size, size_t nmemb, void* userp) {
    *(std::string*)userp +=
        std::string((const char*)buffer, (size * nmemb) / sizeof(char));
    return size * nmemb;
}

} // namespace rest
} // namespace deg