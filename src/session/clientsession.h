/*
 * This file is part of the trojan project.
 * Trojan is an unidentifiable mechanism that helps you bypass GFW.
 * Copyright (C) 2017-2020  The Trojan Authors.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _CLIENTSESSION_H_
#define _CLIENTSESSION_H_

#include "extra/wssessionpool.h"
#include "session.h"
#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>

class ClientSession : public Session
{
  private:
    enum Status
    {
        HANDSHAKE,
        REQUEST,
        CONNECT,
        FORWARD,
        UDP_FORWARD,
        INVALID,
        DESTROY
    } status;
    uint8_t session_id[16]{};
    uint32_t out_recv_package_id = 0;
    uint32_t out_sent_package_id = 0;
    bool is_udp{};
    bool first_packet_recv;
    boost::asio::ip::tcp::socket in_socket;
    boost::asio::ssl::stream<boost::asio::ip::tcp::socket> out_socket;
    std::shared_ptr<WSSessionPool> ws_pool;
    boost::beast::websocket::stream<boost::beast::ssl_stream<boost::beast::tcp_stream>> ws;
    void destroy();
    void in_async_read();
    void in_async_write(const std::string& data);
    void in_recv(const std::string& data);
    void in_sent();
    void out_async_read();
    void out_async_write(const std::string& data);
    void out_recv(const std::string& data);
    void out_sent();
    void udp_async_read();
    void udp_async_write(const std::string& data, const boost::asio::ip::udp::endpoint& endpoint);
    void udp_recv(const std::string& data, const boost::asio::ip::udp::endpoint& endpoint);
    void udp_sent();
    void ws_init();

  public:
    ClientSession(const Config& config, boost::asio::io_context& io_context, boost::asio::ssl::context& ssl_context, decltype(ws_pool) ws_pool);
    boost::asio::ip::tcp::socket& accept_socket() override;
    void start() override;
};

#endif // _CLIENTSESSION_H_
