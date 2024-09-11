#ifndef _SUBSCRIPTION_HPP_
#define _SUBSCRIPTION_HPP_


#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>

#include <websocketpp/common/thread.hpp>
#include <websocketpp/common/memory.hpp>

#include <rapidjson/document.h>
#include <iostream>
#include <string>

namespace deg {

typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

void on_open(websocketpp::connection_hdl hdl, client* c);
void on_message(websocketpp::connection_hdl, client::message_ptr msg);
void on_fail(websocketpp::connection_hdl hdl);
void on_close(websocketpp::connection_hdl hdl);

context_ptr on_tls_init(const char * hostname, websocketpp::connection_hdl);

class Subscription{
    private:
        std::string m_hostname;
        std::string m_uri;
        client m_endpoint;
        websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;
        client::connection_ptr m_conn;
        bool m_connection_launched = false;
    public:
        Subscription(std::string hostname) : m_hostname(hostname), m_uri("wss://" + m_hostname){
            m_endpoint.set_access_channels(websocketpp::log::alevel::all);
            m_endpoint.clear_access_channels(websocketpp::log::alevel::frame_payload);
            m_endpoint.set_error_channels(websocketpp::log::elevel::all);

            m_endpoint.init_asio();
            m_endpoint.start_perpetual();

            m_endpoint.set_tls_init_handler(bind(&on_tls_init, m_hostname.c_str(), deg::_1));

            // Set message, TLS initialization, open, fail, and close handlers
            m_endpoint.set_message_handler(&on_message);

            m_endpoint.set_open_handler(bind(&on_open, deg::_1, &m_endpoint));
            m_endpoint.set_fail_handler(bind(&on_fail, deg::_1));

            m_endpoint.set_close_handler(bind(&on_close, deg::_1));

            m_thread = websocketpp::lib::make_shared<websocketpp::lib::thread>(&client::run, &m_endpoint);

            connect();
        }

        ~Subscription(){
            m_endpoint.stop_perpetual();
            if (m_connection_launched){
                websocketpp::lib::error_code ec;
                m_endpoint.close(m_conn->get_handle(),websocketpp::close::status::going_away,"",ec);
                if (ec) {
                    std::cout << "> Error closing connection: "
                            << ec.message() << std::endl;
                }
            }
            m_thread->join();
        }

        int connect(){
            websocketpp::lib::error_code ec;
            m_conn = m_endpoint.get_connection(m_uri, ec);

            if (ec) {
                std::cout << "> Connect initialization error: " << ec.message() << std::endl;
                return -1;
            }

            m_endpoint.connect(m_conn);

            m_connection_launched = true;

            //con->set_open_handler(bind(&Subscription::on_open,this,&m_endpoint,_1))
            return 0;
        }
};

} // namespace deg

#endif // _SUBSCRIPTION_HPP_