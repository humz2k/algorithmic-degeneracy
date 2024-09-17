#ifndef _AUTH_HPP_
#define _AUTH_HPP_

#include <openssl/sha.h>
#include <openssl/hmac.h>

#include <string>
#include <string_view>
#include <array>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <websocketpp/base64/base64.hpp>

namespace deg{
namespace auth{

typedef unsigned char BYTE;

std::string read_str_from_file(std::string filename);

std::string calc_hmac(std::string_view decodedKey, std::string_view msg);

class Signer{
    private:
        std::string m_secret;
        std::string m_passphrase;
        std::string m_key;

    public:
        Signer(std::string apisecret_path = "apisecret.txt", std::string apipassphrase_path = "apipass.txt", std::string apikey_path = "apikey.txt"){
            auto secret = read_str_from_file(apisecret_path);
            m_secret = websocketpp::base64_decode(secret);
            m_passphrase = read_str_from_file(apipassphrase_path);
            m_key = read_str_from_file(apikey_path);
        }

        std::string sign(std::string message){
            return websocketpp::base64_encode(calc_hmac(m_secret,message));
        }

        const std::string& passphrase(){
            return m_passphrase;
        }

        const std::string& key(){
            return m_key;
        }
};

} // namespace auth
} // namespace deg

#endif // _AUTH_HPP_