#include "restapi.hpp"

namespace deg{
namespace rest{

void init(){
    curl_global_init(CURL_GLOBAL_ALL);
}

void deinit(){
    curl_global_cleanup();
}

size_t write_data(void* buffer, size_t size, size_t nmemb, void* userp){
    const char* test = (const char*)buffer;
    std::string* out = (std::string*)userp;
    out->resize(size * nmemb);
    memcpy(out->data(),buffer,size * nmemb);
    return size * nmemb;
}

} // namespace rest
} // namespace deg