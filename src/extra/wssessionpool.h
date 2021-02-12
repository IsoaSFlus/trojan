#ifndef _WSSESSIONPOOL_H_
#define _WSSESSIONPOOL_H_

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

class WSSessionPool : public std::enable_shared_from_this<WSSessionPool>
{

    boost::asio::io_context& io_context;
    boost::asio::ssl::context& ssl_context;

  public:
    // Resolver and socket require an io_context
    explicit WSSessionPool(boost::asio::io_context& ioc, boost::asio::ssl::context& ssl_context);
    boost::asio::ip::tcp::endpoint get_endpoint();
};

#endif // _WSSESSIONPOOL_H_
