#include "auth.hpp"
#include "circular_buffer.hpp"
#include "restapi.hpp"
#include "subscription.hpp"
#include <fstream>
#include <iostream>

std::ofstream csv_data;

class Handler : public deg::TickerSubscriptionHandler {
  private:
    deg::circular_buffer<double, 5> m_price_moving_avg;
    deg::circular_buffer<double, 20> m_price_moving_avg2;

  public:
    Handler() : deg::TickerSubscriptionHandler("DOGE-USD") {}
    void on_ticker(const deg::ticker_data& data) override {
        using namespace date;
        double avg = m_price_moving_avg.average();
        double spread = data.best_ask - data.best_bid;
        double diff = data.price - avg;
        double rel_diff = diff / spread;
        double lerp = (data.price - data.best_bid) / spread;
        auto time_now = std::chrono::system_clock::now();
        std::chrono::nanoseconds time_diff = time_now - data.tp;

        csv_data << avg << "," << data.price << "," << diff << "," << rel_diff
                 << "," << spread << "," << lerp << "," << data.tp << ","
                 << time_now << "," << time_diff.count() << "," << data.best_bid
                 << "," << data.best_ask << "," << m_price_moving_avg2.average()
                 << "\n";

        std::cout << "#tick:\n"
                  << "   price moving avg: " << avg << "\n   diff = " << diff
                  << "\n   rel_diff = " << rel_diff
                  << "\n   spread = " << spread << "\n   lerp = " << lerp
                  << "\n   trade_time = " << data.tp
                  << "\n   time_now = " << time_now
                  << "\n   time_diff = " << time_diff.count() << std::endl;

        // data.print();

        m_price_moving_avg.insert(data.price);
        m_price_moving_avg2.insert(data.price);
    }
};

int main() {
    deg::rest::CurlManager curl_manager;

    deg::rest::GetRequest time_req("/time");
    std::cout << "yeet: " << time_req.go() << std::endl;

    deg::rest::TradeAPI trade_api;

    std::cout << trade_api.get_server_time().count() << std::endl;
    std::cout << "last_rtt = " << trade_api.last_rtt() << std::endl;
    std::cout << "avg_rtt = " << trade_api.avg_rtt() << std::endl;

    trade_api.get_orders();
    std::cout << "last_rtt = " << trade_api.last_rtt() << std::endl;
    std::cout << "avg_rtt = " << trade_api.avg_rtt() << std::endl;

    /*csv_data.open ("doge_data2.csv");
    csv_data <<
    "mvavg,price,mvavgdiff,relmvavgdiff,spread,lerp,timetrade,timenow,latency,bid,ask,longermvavgprice\n";
    deg::Subscription<Handler>
    subscription("ws-feed-public.sandbox.exchange.coinbase.com ");

    while (true) {
        std::string command;
        std::getline(std::cin, command);
        if (command == "quit") {
            break;
        }
    }

    csv_data.close();*/

    return 0;
}