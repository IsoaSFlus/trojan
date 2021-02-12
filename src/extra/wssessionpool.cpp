#include "wssessionpool.h"
#include <string>

WSSessionPool::WSSessionPool(boost::asio::io_context& ioc, boost::asio::ssl::context& ssl_context)
  : io_context(ioc)
  , ssl_context(ssl_context)
{}

boost::asio::ip::tcp::endpoint
WSSessionPool::get_endpoint()
{
    boost::system::error_code ec;
    boost::asio::ip::address addr = boost::asio::ip::address::from_string("104.19.106.212", ec);
    return boost::asio::ip::tcp::endpoint(addr, 443);
}
