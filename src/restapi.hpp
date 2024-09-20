#ifndef _REST_API_HPP_
#define _REST_API_HPP_

#include "auth.hpp"
#include "circular_buffer.hpp"
#include "types.hpp"
#include <cassert>
#include <chrono>
#include <curl/curl.h>
#include <format>
#include <future>
#include <iomanip>
#include <iostream>
#include <rapidjson/document.h>
#include <sstream>
#include <string>
// #include <async>

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
        m_out.clear();
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

class AuthenticatedPostRequest {
  private:
    struct curl_slist* m_headers = NULL;
    double m_last_rtt = 0;
    deg::circular_buffer<double, 5> m_rtt_buff;

    deg::auth::JWTCreator m_jwt;
    std::string m_endpoint;
    std::string m_url;

  protected:
    CURL* m_curl;
    // CURLM* m_curl_multi;
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
    AuthenticatedPostRequest(std::string endpoint,
                             std::string url = "api.coinbase.com",
                             std::string protocol = "https")
        : m_curl(curl_easy_init()), m_endpoint(endpoint), m_url(url) {
        std::string uri =
            protocol + "://" + url + "/api/v3/brokerage" + endpoint;
        assert(m_curl);
        curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(m_curl, CURLOPT_URL, uri.c_str());
        curl_easy_setopt(m_curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(m_curl, CURLOPT_DEFAULT_PROTOCOL, protocol.c_str());
        curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, deg::rest::write_data);
        curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &m_out);
        add_header("Content-Type: application/json");
    }

    AuthenticatedPostRequest(std::string endpoint, std::string url,
                             std::string protocol, std::string key_name,
                             std::string key_secret)
        : m_jwt(key_name, key_secret), m_curl(curl_easy_init()),
          m_endpoint(endpoint), m_url(url) {
        std::string uri =
            protocol + "://" + url + "/api/v3/brokerage" + endpoint;
        assert(m_curl);
        curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(m_curl, CURLOPT_URL, uri.c_str());
        curl_easy_setopt(m_curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(m_curl, CURLOPT_DEFAULT_PROTOCOL, protocol.c_str());
        curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, deg::rest::write_data);
        curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &m_out);
        add_header("Content-Type: application/json");
    }

    virtual std::string send_request(std::string post_headers) {
        m_out.clear();
        auto jwt = m_jwt.create("POST", "/api/v3/brokerage" + m_endpoint);
        reset_headers();
        add_header("Content-Type: application/json");
        add_header("Authorization: Bearer " + jwt);
        curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, post_headers.c_str());
        auto time_before = std::chrono::system_clock::now();
        m_res = curl_easy_perform(m_curl);
        auto time_after = std::chrono::system_clock::now();
        update_rtt(time_before, time_after);
        return m_out;
    }

    std::string go(std::string post_headers) {
        return this->send_request(post_headers);
    }

    double last_rtt() { return m_last_rtt; }

    double avg_rtt() { return m_rtt_buff.average(); }

    ~AuthenticatedPostRequest() {
        curl_slist_free_all(m_headers);
        curl_easy_cleanup(m_curl);
        // curl_multi_cleanup(m_curl_multi);
    }
};

class AuthenticatedGetRequest : public GetRequest {
  private:
    deg::auth::JWTCreator m_jwt;
    std::string m_url;
    std::string m_endpoint;

  public:
    AuthenticatedGetRequest(std::string endpoint, std::string extras = "",
                            std::string url = "api.coinbase.com",
                            std::string protocol = "https")
        : GetRequest(endpoint + extras, url, protocol), m_url(url),
          m_endpoint(endpoint) {}

    std::string send_request() override {
        m_out.clear();
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
    std::string m_key_name;
    std::string m_key_secret;
    GetRequest m_time_req = GetRequest("/time");
    AuthenticatedGetRequest m_order_req = AuthenticatedGetRequest(
        "/orders/historical/batch", "?order_status=OPEN");
    AuthenticatedPostRequest m_order_placer =
        AuthenticatedPostRequest("/orders");
    double m_last_rtt = 0;
    deg::circular_buffer<double, 5> m_avg_rtt;
    long long m_order_id_start;

    void update_rtt(double rtt) {
        m_last_rtt = rtt;
        m_avg_rtt.insert(rtt);
    }

    std::string get_fresh_order_id() {
        m_order_id_start++;
        return std::to_string(m_order_id_start);
    }

  public:
    TradeAPI(std::string key_name_path = "apikeyname.txt",
             std::string key_secret_path = "apiprivkey.txt")
        : m_key_name(auth::read_str_from_file(key_name_path)),
          m_key_secret(auth::read_str_from_file(key_secret_path)),
          m_order_id_start(get_server_time().count()) {}

    std::chrono::milliseconds get_server_time() {
        rapidjson::Document document;
        update_rtt(m_time_req.go_json(document));
        return std::chrono::milliseconds(
            std::stoll(document["epochMillis"].GetString()));
    }

    void get_orders() { std::cout << "yes: " << m_order_req.go() << std::endl; }

    // returns orderid if successful, and NULL if not
    std::string place_market_order(const std::string& symbol, Side side,
                                   Unit unit, Value size,
                                   int unit_decimals = 0) {
        std::stringstream ss;
        if (unit_decimals == 0) {
            unit_decimals = (unit == QUOTE) ? 5 : 8;
        }
        ss << std::fixed << std::setprecision(unit_decimals) << size;
        std::string order_id = get_fresh_order_id();
        std::string out =
            "{\"client_order_id\":\"" + order_id + "\",\"product_id\":\"" +
            symbol + "\",\"side\":\"" + ((side == BUY) ? "BUY" : "SELL") +
            "\",\"order_configuration\":{\"market_market_ioc\":{\"" +
            ((unit == QUOTE) ? "quote_size" : "base_size") + "\":\"" +
            ss.str() + "\"}}}";
        // std::cout << out << std::endl;
        rapidjson::Document doc;
        doc.Parse(m_order_placer.go(out).c_str());
        return doc["success"].GetBool() ? order_id : "NULL";
    }

    // returns orderid if successful, and NULL if not
    // this is in units of the FIRST QUANTITY OF THE PAIR!!!!!!!!!!!!!
    void place_limit_order(const std::string& symbol, Side side,
                           Value base_size, Value limit_price,
                           int price_decimals = 5, bool post_only = true) {
        std::stringstream base_size_ss;
        base_size_ss << std::fixed << std::setprecision(8) << base_size;

        std::stringstream limit_price_ss;
        limit_price_ss << std::fixed << std::setprecision(price_decimals)
                       << limit_price;

        std::string order_id = get_fresh_order_id();
        std::string out =
            "{\"client_order_id\":\"" + order_id + "\",\"product_id\":\"" +
            symbol + "\",\"side\":\"" + ((side == BUY) ? "BUY" : "SELL") +
            "\",\"order_configuration\":{\"limit_limit_gtc\":{\"" +
            "base_size" + "\":\"" + base_size_ss.str() +
            "\",\"limit_price\":\"" + limit_price_ss.str() +
            "\",\"post_only\":" + (post_only ? "true" : "false") + "}}}";
        // std::cout << out << std::endl;
        rapidjson::Document doc;
        AuthenticatedPostRequest order_placer(
            "/orders", "api.coinbase.com", "https", m_key_name, m_key_secret);
        std::string response = order_placer.go(out);
        doc.Parse(response.c_str());

        if (!doc["success"].GetBool())
            std::cout << response << std::endl;

        // return doc["success"].GetBool() ? order_id : "NULL";
    }

    double last_rtt() { return m_last_rtt; }

    double avg_rtt() { return m_avg_rtt.average(); }
};

} // namespace rest
} // namespace deg

#endif // _REST_API_HPP_