#ifndef _AUTH_HPP_
#define _AUTH_HPP_

#include <iostream>
#include <jwt-cpp/jwt.h>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <sstream>
#include <string>

namespace deg {
namespace auth {

std::string read_str_from_file(std::string filename);

class JWTCreator {
  private:
    std::string m_key_name;
    std::string m_key_secret;

  public:
    JWTCreator(std::string key_name_path = "apikeyname.txt",
               std::string key_secret_path = "apiprivkey.txt")
        : m_key_name(read_str_from_file(key_name_path)),
          m_key_secret(read_str_from_file(key_secret_path)) {}

    std::string create(std::string request_method, std::string request_path,
                       std::string url = "api.coinbase.com") {
        std::string uri = request_method + " " + url + request_path;

        // Generate a random nonce
        unsigned char nonce_raw[16];
        RAND_bytes(nonce_raw, sizeof(nonce_raw));
        std::string nonce(reinterpret_cast<char*>(nonce_raw),
                          sizeof(nonce_raw));

        // Create JWT token
        auto token = jwt::create()
                         .set_subject(m_key_name)
                         .set_issuer("cdp")
                         .set_not_before(std::chrono::system_clock::now())
                         .set_expires_at(std::chrono::system_clock::now() +
                                         std::chrono::seconds{120})
                         .set_payload_claim("uri", jwt::claim(uri))
                         .set_header_claim("kid", jwt::claim(m_key_name))
                         .set_header_claim("nonce", jwt::claim(nonce))
                         .sign(jwt::algorithm::es256(m_key_name, m_key_secret));

        return token;
    };
};

} // namespace auth
} // namespace deg

#endif // _AUTH_HPP_