#ifndef _WSSESSION_H_
#define _WSSESSION_H_

#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

class WSSession : public std::enable_shared_from_this<WSSession>
{
    boost::asio::ip::tcp::resolver resolver_;
    boost::beast::websocket::stream<boost::beast::ssl_stream<boost::beast::tcp_stream>> ws_;
    boost::beast::flat_buffer buffer_;
    std::string host_;
    std::string text_;

  public:
    // Resolver and socket require an io_context
    explicit WSSession(boost::asio::io_context& ioc, boost::asio::ssl::context& ctx);

    void run(char const* host, char const* port, char const* text);
    void on_resolve(boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type results);
    void on_connect(boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type::endpoint_type ep);
    void on_ssl_handshake(boost::beast::error_code ec);
    void on_handshake(boost::beast::error_code ec);
    void on_write(boost::beast::error_code ec, std::size_t bytes_transferred);
    void on_read(boost::beast::error_code ec, std::size_t bytes_transferred);
    void on_close(boost::beast::error_code ec);
};

#endif // _WSSESSION_H_
