#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <rapidjson/document.h>
#include <iostream>
#include "subscription.hpp"

class Handler : public deg::TickerSubscriptionHandler{
    public:
    Handler() : deg::TickerSubscriptionHandler("BTC-USD"){}
    void on_ticker(const deg::ticker_data& data) override{
        //std::cout << "price: " << data.price << std::endl;
        data.print();
    }
};

int main(){
    using namespace std::chrono_literals;
    deg::Subscription<Handler> subscription("ws-feed.exchange.coinbase.com");
    std::this_thread::sleep_for(10000ms);
    //subscription.close();
}