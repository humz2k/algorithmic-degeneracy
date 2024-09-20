#ifndef _SUBSCRIPTION_HPP_
#define _SUBSCRIPTION_HPP_

#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>

#include <websocketpp/common/memory.hpp>
#include <websocketpp/common/thread.hpp>

#include <iostream>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <string>
#include <vector>

#include "auth.hpp"

#include "date/date.h"

namespace deg {

typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context>
    context_ptr;

using websocketpp::lib::bind;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;

context_ptr on_tls_init(const char* hostname, websocketpp::connection_hdl);

template <typename handler_t> class Subscription {
  private:
    std::string m_hostname;
    std::string m_uri;
    client m_endpoint;
    websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;
    client::connection_ptr m_conn;
    bool m_connection_launched = false;
    handler_t m_handler;

    void init() {
        m_endpoint.set_access_channels(websocketpp::log::alevel::all);
        m_endpoint.clear_access_channels(
            websocketpp::log::alevel::frame_payload);
        m_endpoint.set_error_channels(websocketpp::log::elevel::all);

        m_endpoint.init_asio();
        m_endpoint.start_perpetual();

        m_endpoint.set_tls_init_handler(
            bind(&on_tls_init, m_hostname.c_str(), deg::_1));

        m_endpoint.set_message_handler(
            bind(&handler_t::_on_message, &m_handler, deg::_1, deg::_2));
        m_endpoint.set_open_handler(
            bind(&handler_t::_on_open, &m_handler, deg::_1, &m_endpoint));
        m_endpoint.set_fail_handler(
            bind(&handler_t::on_fail, &m_handler, deg::_1));
        m_endpoint.set_close_handler(
            bind(&handler_t::on_close, &m_handler, deg::_1));

        m_thread = websocketpp::lib::make_shared<websocketpp::lib::thread>(
            &client::run, &m_endpoint);

        connect();
    }

  public:
    Subscription(std::string hostname)
        : m_hostname(hostname), m_uri("wss://" + m_hostname) {
        init();
    }

    template <typename... Args>
    Subscription(std::string hostname, Args... args)
        : m_hostname(hostname), m_uri("wss://" + m_hostname),
          m_handler(args...) {
        init();
    }

    ~Subscription() {
        m_endpoint.stop_perpetual();
        if (m_connection_launched) {
            websocketpp::lib::error_code ec;
            m_endpoint.close(m_conn->get_handle(),
                             websocketpp::close::status::going_away, "", ec);
            if (ec) {
                std::cout << "> Error closing connection: " << ec.message()
                          << std::endl;
            }
        }
        m_thread->join();
    }

    int connect() {
        websocketpp::lib::error_code ec;
        m_conn = m_endpoint.get_connection(m_uri, ec);

        if (ec) {
            std::cout << "> Connect initialization error: " << ec.message()
                      << std::endl;
            return -1;
        }

        m_endpoint.connect(m_conn);

        m_connection_launched = true;

        return 0;
    }
};

class SubscriptionHandler {
  public:
    void _on_open(websocketpp::connection_hdl hdl, client* c) {
        this->on_open(hdl, c);
    }

    virtual void on_open(websocketpp::connection_hdl hdl, client* c) {
        std::cout << "WebSocket connection opened!" << std::endl;
    }

    void on_fail(websocketpp::connection_hdl hdl) {
        std::cout << "WebSocket connection failed!" << std::endl;
    }

    void on_close(websocketpp::connection_hdl hdl) {
        std::cout << "WebSocket connection closed!" << std::endl;
    }

    void _on_message(websocketpp::connection_hdl hdl,
                     deg::client::message_ptr msg) {
        rapidjson::Document document;
        std::string payload = msg->get_payload();
        std::cout << "recieved: " << payload << std::endl;
        document.Parse(payload.c_str());
        this->on_message(hdl, document);
    }

    virtual void on_message(websocketpp::connection_hdl,
                            const rapidjson::Document& document) = 0;
};

struct ticker_data {
    std::string product_id;
    double price;
    double open_24h;
    double volume_24h;
    double low_24h;
    double high_24h;
    double volume_30d;
    double best_bid;
    double best_bid_size;
    double best_ask;
    double best_ask_size;
    bool buy;
    std::string time;
    // uint32_t trade_id;
    double last_size;
    std::chrono::system_clock::time_point tp;

    ticker_data(const rapidjson::Document& document) {
        product_id = document["product_id"].GetString();
        price = std::stof(document["price"].GetString());
        open_24h = std::stof(document["open_24h"].GetString());
        volume_24h = std::stof(document["volume_24h"].GetString());
        low_24h = std::stof(document["low_24h"].GetString());
        high_24h = std::stof(document["high_24h"].GetString());
        volume_30d = std::stof(document["volume_30d"].GetString());
        best_bid = std::stof(document["best_bid"].GetString());
        best_bid_size = std::stof(document["best_bid_size"].GetString());
        best_ask = std::stof(document["best_ask"].GetString());
        best_ask_size = std::stof(document["best_ask_size"].GetString());
        buy = strcmp(document["side"].GetString(), "buy") == 0;
        time = document["time"].GetString();
        // trade_id = document["trade_id"].GetInt64();
        last_size = std::stof(document["last_size"].GetString());
        // std::tm tm = {};
        std::istringstream in(time);
        in >> date::parse("%Y-%m-%dT%T", tp);
        // ss >> std::get_time(&tm, "%Y:%m:%dT%H:%M:%S.")
    }

    void print() const {
        std::cout << "ID: " << product_id << "\n";
        std::cout << "   PRICE: " << price << "\n";
        std::cout << "   OPEN24H: " << open_24h << "\n";
        std::cout << "   VOL24H: " << volume_24h << "\n";
        std::cout << "   LOW24H: " << low_24h << "\n";
        std::cout << "   HIGH24H: " << high_24h << "\n";
        std::cout << "   VOL30D: " << volume_30d << "\n";
        std::cout << "   BEST_BID: " << best_bid << "\n";
        std::cout << "   BEST_BID_SIZE: " << best_bid_size << "\n";
        std::cout << "   BEST_ASK: " << best_ask << "\n";
        std::cout << "   BEST_ASK_SIZE: " << best_ask_size << "\n";
        std::cout << "   BUY: " << (buy ? "true" : "false") << "\n";
        std::cout << "   TIME: " << time << "\n";
        std::cout << "   LAST_SIZE: " << last_size << std::endl;
    }
};

class TickerSubscriptionHandler : public SubscriptionHandler {
  private:
    std::string m_id;

  public:
    TickerSubscriptionHandler(std::string id) : m_id(id) {}

    void on_open(websocketpp::connection_hdl hdl, client* c) override {
        std::cout << "WebSocket ticker connection opened!" << std::endl;
        websocketpp::lib::error_code ec;

        client::connection_ptr con = c->get_con_from_hdl(hdl, ec);
        if (ec) {
            std::cout << "Failed to get connection pointer: " << ec.message()
                      << std::endl;
            return;
        }
        std::string payload = "{\"type\":\"subscribe\", \"product_ids\":[\"" +
                              m_id +
                              "\"], \"channels\":[\"ticker\",\"heartbeat\"]}";
        std::cout << "payload : " << payload << std::endl;
        c->send(con, payload, websocketpp::frame::opcode::text);
    }

    void on_message(websocketpp::connection_hdl,
                    const rapidjson::Document& document) override {
        if (document["type"] == "ticker") {
            ticker_data data(document);
            this->on_ticker(data);
        }
    }

    virtual void on_ticker(const ticker_data& data) = 0;
};

class UserSubscriptionHandler : public SubscriptionHandler {
  private:
    std::string m_id;
    deg::auth::JWTCreator m_jwt;

  public:
    UserSubscriptionHandler(std::string id) : m_id(id) {}

    void on_open(websocketpp::connection_hdl hdl, client* c) override {
        std::cout << "WebSocket ticker connection opened!" << std::endl;
        websocketpp::lib::error_code ec;

        client::connection_ptr con = c->get_con_from_hdl(hdl, ec);
        if (ec) {
            std::cout << "Failed to get connection pointer: " << ec.message()
                      << std::endl;
            return;
        }
        std::string payload =
            "{\"type\":\"subscribe\",\"channel\":\"user\",\"product_ids\":[\"" +
            m_id + "\"],\"jwt\":\"" + m_jwt.create() + "\"}";
        std::cout << "payload : " << payload << std::endl;
        c->send(con, payload, websocketpp::frame::opcode::text);
    }

    void on_message(websocketpp::connection_hdl,
                    const rapidjson::Document& document) override {
        if (document["channel"] == "user") {
            std::cout << "recieved user!" << std::endl;
            // ticker_data data(document);
            // std::cout << document << std::endl;
            // this->on_ticker(data);
        }
    }

    // virtual void on_ticker(const ticker_data& data) = 0;
};

} // namespace deg

#endif // _SUBSCRIPTION_HPP_