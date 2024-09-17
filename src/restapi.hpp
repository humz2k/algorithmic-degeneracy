#ifndef _REST_API_HPP_
#define _REST_API_HPP_

#include "auth.hpp"
#include "circular_buffer.hpp"
#include <cassert>
#include <chrono>
#include <curl/curl.h>
#include <iostream>
#include <rapidjson/document.h>
#include <string>

namespace deg {
namespace rest {
void init();
void deinit();
size_t write_data(void* buffer, size_t size, size_t nmemb, void* userp);

class CurlManager {
  public:
    CurlManager() { init(); }
    ~CurlManager() { deinit(); }
};

class GetRequest {
  private:
    struct curl_slist* m_headers = NULL;
    double m_last_rtt = 0;
    deg::circular_buffer<double, 5> m_rtt_buff;

  protected:
    CURL* m_curl;
    CURLcode m_res;
    std::string m_out;

    void reset_headers() {
        if (m_headers) {
            curl_slist_free_all(m_headers);
        }
        m_headers = NULL;
        curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, m_headers);
    }

    void add_header(std::string s) {
        m_headers = curl_slist_append(m_headers, s.c_str());
        curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, m_headers);
    }

    void update_rtt(const std::chrono::system_clock::time_point& t1,
                    const std::chrono::system_clock::time_point& t2) {
        std::chrono::nanoseconds duration = t2 - t1;
        m_last_rtt = ((double)((duration).count())) * 1e-6;
        m_rtt_buff.insert(m_last_rtt);
    }

  public:
    GetRequest(std::string endpoint, std::string url = "api.coinbase.com",
               std::string protocol = "https")
        : m_curl(curl_easy_init()) {
        std::string uri =
            protocol + "://" + url + "/api/v3/brokerage" + endpoint;
        assert(m_curl);
        curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, "GET");
        curl_easy_setopt(m_curl, CURLOPT_URL, uri.c_str());
        curl_easy_setopt(m_curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(m_curl, CURLOPT_DEFAULT_PROTOCOL, protocol.c_str());
        curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, deg::rest::write_data);
        curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &m_out);
        add_header("Content-Type: application/json");
    }

    virtual std::string send_request() {
        auto time_before = std::chrono::system_clock::now();
        m_res = curl_easy_perform(m_curl);
        auto time_after = std::chrono::system_clock::now();
        update_rtt(time_before, time_after);
        return m_out;
    }

    std::string go() { return this->send_request(); }

    double go_json(rapidjson::Document& document) {
        this->send_request();
        document.Parse(m_out.c_str());
        return m_last_rtt;
    }

    ~GetRequest() {
        curl_slist_free_all(m_headers);
        curl_easy_cleanup(m_curl);
    }

    double last_rtt() { return m_last_rtt; }

    double avg_rtt() { return m_rtt_buff.average(); }
};

class AuthenticatedGetRequest : public GetRequest {
  private:
    deg::auth::JWTCreator m_jwt;
    std::string m_url;
    std::string m_endpoint;

  public:
    AuthenticatedGetRequest(std::string endpoint,
                            std::string url = "api.coinbase.com",
                            std::string protocol = "https")
        : GetRequest(endpoint, url, protocol), m_url(url),
          m_endpoint(endpoint) {}

    std::string send_request() override {
        auto jwt = m_jwt.create("GET", "/api/v3/brokerage" + m_endpoint);
        reset_headers();
        add_header("Content-Type: application/json");
        add_header("Authorization: Bearer " + jwt);
        auto time_before = std::chrono::system_clock::now();
        m_res = curl_easy_perform(m_curl);
        auto time_after = std::chrono::system_clock::now();
        update_rtt(time_before, time_after);
        return m_out;
    }
};

class TradeAPI {
  private:
    GetRequest m_time_req = GetRequest("/time");
    AuthenticatedGetRequest m_order_req =
        AuthenticatedGetRequest("/orders/historical/batch");
    double m_last_rtt = 0;
    deg::circular_buffer<double, 5> m_avg_rtt;

    void update_rtt(double rtt) {
        m_last_rtt = rtt;
        m_avg_rtt.insert(rtt);
    }

  public:
    std::chrono::milliseconds get_server_time() {
        rapidjson::Document document;
        update_rtt(m_time_req.go_json(document));
        return std::chrono::milliseconds(
            std::stoll(document["epochMillis"].GetString()));
    }

    void get_orders() { std::cout << m_order_req.go() << std::endl; }

    double last_rtt() { return m_last_rtt; }

    double avg_rtt() { return m_avg_rtt.average(); }
};

} // namespace rest
} // namespace deg

#endif // _REST_API_HPP_