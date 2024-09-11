#include "subscription.hpp"
#include <iostream>
#include <rapidjson/document.h>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>

class Handler : public deg::TickerSubscriptionHandler {
  public:
    Handler() : deg::TickerSubscriptionHandler("BTC-USD") {}
    void on_ticker(const deg::ticker_data& data) override {
        data.print();
    }
};

int main() {
    using namespace std::chrono_literals;
    deg::Subscription<Handler> subscription("ws-feed.exchange.coinbase.com");

    while (true) {
        std::string command;
        std::getline(std::cin, command);
        if (command == "quit") {
            break;
        }
    }
}