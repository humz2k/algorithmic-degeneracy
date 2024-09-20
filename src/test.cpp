#include "auth.hpp"
#include "circular_buffer.hpp"
#include "restapi.hpp"
#include "subscription.hpp"
#include <fstream>
#include <iostream>

class UserHandler : public deg::UserSubscriptionHandler {
  public:
    UserHandler() : deg::UserSubscriptionHandler("BTC-USD") {}
};

class Handler : public deg::TickerSubscriptionHandler {
  private:
    // deg::circular_buffer<double, 5> m_price_moving_avg;
    // deg::circular_buffer<double, 20> m_price_moving_avg2;
    deg::rest::TradeAPI trade_api;
    int exposure = 0;
    double spread = 2;

  public:
    Handler() : deg::TickerSubscriptionHandler("BTC-USD") {}
    void on_ticker(const deg::ticker_data& data) override {
        using namespace date;
        std::cout << "ask: " << std::to_string(data.best_ask) << ", bid: " << std::to_string(data.best_bid)
                  << std::endl;
        /*if ((exposure < 10) && ((data.best_ask - data.best_bid) >= spread)){
            std::cout << "placing orders" << std::endl;
            //if ((exposure % 2) == 0)
            //std::async(&deg::rest::TradeAPI::place_limit_order,&trade_api,"BTC-USD",deg::BUY,0.00001,data.best_bid
        + spread,2,true);
            //else
            std::async(&deg::rest::TradeAPI::place_limit_order,&trade_api,"BTC-USD",deg::SELL,0.00001,data.best_ask
        - spread,2,true);
            //std::cout <<
        trade_api.place_limit_order("BTC-USD",deg::SELL,0.00001,data.best_bid,2)
        << std::endl; exposure++;
        }*/
        /*double avg = m_price_moving_avg.average();
        double spread = data.best_ask - data.best_bid;
        double diff = data.price - avg;
        double rel_diff = diff / spread;
        double lerp = (data.price - data.best_bid) / spread;
        auto time_now = std::chrono::system_clock::now();
        std::chrono::nanoseconds time_diff = time_now - data.tp;

        std::cout << "#tick:\n"
                  << "   price moving avg: " << avg << "\n   diff = " << diff
                  << "\n   rel_diff = " << rel_diff
                  << "\n   spread = " << spread << "\n   lerp = " << lerp
                  << "\n   trade_time = " << data.tp
                  << "\n   time_now = " << time_now
                  << "\n   time_diff = " << time_diff.count() << std::endl;

        m_price_moving_avg.insert(data.price);
        m_price_moving_avg2.insert(data.price);*/
    }
};

int main() {
    deg::rest::CurlManager curl_manager;

    // deg::rest::GetRequest time_req("/time");
    // std::cout << "yeet: " << time_req.go() << std::endl;

    /*deg::rest::AuthenticatedPostRequest place_order("/orders");
    std::cout << "order?: " <<
    place_order.go("{\"client_order_id\":\"1\",\"product_id\":\"BTC-USD\",\"side\":\"BUY\",\"order_configuration\":{\"market_market_ioc\":{\"quote_size\":\"1.00\"}}}")
    << std::endl; std::cout << place_order.last_rtt() << std::endl;*/
    // deg::rest::TradeAPI trade_api;

    // std::cout <<
    // trade_api.place_market_order("BTC-USD",deg::SELL,deg::BASE,0.00001548) <<
    // std::endl; std::cout <<
    // trade_api.place_limit_order("BTC-USD",deg::SELL,0.00001548,63901.00) <<
    // std::endl;
    /*std::cout << trade_api.get_server_time().count() << std::endl;
    std::cout << "last_rtt = " << trade_api.last_rtt() << std::endl;
    std::cout << "avg_rtt = " << trade_api.avg_rtt() << std::endl;

    trade_api.get_orders();
    std::cout << "last_rtt = " << trade_api.last_rtt() << std::endl;
    std::cout << "avg_rtt = " << trade_api.avg_rtt() << std::endl;*/

    //deg::Subscription<Handler> subscription("ws-feed.exchange.coinbase.com");
    deg::Subscription<UserHandler> subscription1(
        "advanced-trade-ws-user.coinbase.com");

    while (true) {
        std::string command;
        std::getline(std::cin, command);
        if (command == "quit") {
            break;
        }
    }

    return 0;
}