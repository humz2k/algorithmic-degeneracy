#include "subscription.hpp"
#include <iostream>
#include <fstream>
#include <rapidjson/document.h>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>

std::ofstream csv_data;


template<typename T, int n>
class circular_buffer{
    private:
        T m_buff[n] = {0};
        int m_count = 0;
    public:
        void insert(T val){
            m_buff[m_count] = val;
            m_count = (m_count + 1)%n;
        }

        T average(){
            T sum = 0;
            for (int i = 0; i < n; i++){
                sum += m_buff[i];
            }
            return sum / (T)n;
        }
};

class Handler : public deg::TickerSubscriptionHandler {
  private:
    circular_buffer<double,5> m_price_moving_avg;
    circular_buffer<double,20> m_price_moving_avg2;
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

        csv_data
        << avg  << "," << data.price
        << "," << diff
        << "," << rel_diff
        << "," << spread
        << "," << lerp
        << "," << data.tp
        << "," << time_now
        << "," << time_diff.count()
        << "," << data.best_bid
        << "," << data.best_ask
        << "," << m_price_moving_avg2.average()
        << "\n";

        std::cout << "#tick:\n"
        << "   price moving avg: " << avg
        << "\n   diff = " << diff
        << "\n   rel_diff = " << rel_diff
        << "\n   spread = " << spread
        << "\n   lerp = " << lerp
        << "\n   trade_time = " << data.tp
        << "\n   time_now = " << time_now
        << "\n   time_diff = " << time_diff.count()
        << std::endl;

        //data.print();

        m_price_moving_avg.insert(data.price);
        m_price_moving_avg2.insert(data.price);

    }
};

int main() {
    using namespace std::chrono_literals;
    csv_data.open ("doge_data2.csv");
    csv_data << "mvavg,price,mvavgdiff,relmvavgdiff,spread,lerp,timetrade,timenow,latency,bid,ask,longermvavgprice\n";
    deg::Subscription<Handler> subscription("ws-feed-public.sandbox.exchange.coinbase.com ");

    while (true) {
        std::string command;
        std::getline(std::cin, command);
        if (command == "quit") {
            break;
        }
    }

    csv_data.close();
}