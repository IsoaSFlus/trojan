#include "wssession.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

//------------------------------------------------------------------------------

// Report a failure
void
fail(beast::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

// Resolver and socket require an io_context
WSSession::WSSession(net::io_context& ioc, ssl::context& ctx)
  : resolver_(net::make_strand(ioc))
  , ws_(net::make_strand(ioc), ctx)
{}

// Start the asynchronous operation
void
WSSession::run(char const* host, char const* port, char const* text)
{
    // Save these for later
    host_ = host;
    text_ = text;

    // Look up the domain name
    resolver_.async_resolve(host, port, beast::bind_front_handler(&WSSession::on_resolve, shared_from_this()));
}

void
WSSession::on_resolve(beast::error_code ec, tcp::resolver::results_type results)
{
    if (ec)
        return fail(ec, "resolve");

    // Set a timeout on the operation
    beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

    // Make the connection on the IP address we get
    // from a lookup
    beast::get_lowest_layer(ws_).async_connect(results, beast::bind_front_handler(&WSSession::on_connect, shared_from_this()));
}

void
WSSession::on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep)
{
    if (ec)
        return fail(ec, "connect");

    // Update the host_ string. This will provide
    // the value of the Host HTTP header during the
    // WebSocket handshake. See
    // https://tools.ietf.org/html/rfc7230#section-5.4
    host_ += ':' + std::to_string(ep.port());

    // Set a timeout on the operation
    beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

    // Set SNI Hostname (many hosts need this to
    // handshake successfully)
    if (!SSL_set_tlsext_host_name(ws_.next_layer().native_handle(), host_.c_str())) {
        ec = beast::error_code(static_cast<int>(::ERR_get_error()), net::error::get_ssl_category());
        return fail(ec, "connect");
    }

    // Perform the SSL handshake
    ws_.next_layer().async_handshake(ssl::stream_base::client, beast::bind_front_handler(&WSSession::on_ssl_handshake, shared_from_this()));
}

void
WSSession::on_ssl_handshake(beast::error_code ec)
{
    if (ec)
        return fail(ec, "ssl_handshake");

    // Turn off the timeout on the tcp_stream,
    // because the websocket stream has its own
    // timeout system.
    beast::get_lowest_layer(ws_).expires_never();

    // Set suggested timeout settings for the
    // websocket
    ws_.set_option(websocket::stream_base::timeout::suggested(beast::role_type::client));

    // Set a decorator to change the User-Agent of
    // the handshake
    ws_.set_option(websocket::stream_base::decorator([](websocket::request_type& req) {
        req.set(http::field::user_agent,
                std::string(BOOST_BEAST_VERSION_STRING) + " websocket-client-"
                                                          "async-ssl");
    }));

    // Perform the websocket handshake
    ws_.async_handshake(host_, "/", beast::bind_front_handler(&WSSession::on_handshake, shared_from_this()));
}

void
WSSession::on_handshake(beast::error_code ec)
{
    if (ec)
        return fail(ec, "handshake");

    // Send the message
    ws_.async_write(net::buffer(text_), beast::bind_front_handler(&WSSession::on_write, shared_from_this()));
}

void
WSSession::on_write(beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if (ec)
        return fail(ec, "write");

    // Read a message into our buffer
    ws_.async_read(buffer_, beast::bind_front_handler(&WSSession::on_read, shared_from_this()));
}

void
WSSession::on_read(beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if (ec)
        return fail(ec, "read");

    // Close the WebSocket connection
    ws_.async_close(websocket::close_code::normal, beast::bind_front_handler(&WSSession::on_close, shared_from_this()));
}

void
WSSession::on_close(beast::error_code ec)
{
    if (ec)
        return fail(ec, "close");

    // If we get here then the connection is closed
    // gracefully

    // The make_printable() function helps print a
    // ConstBufferSequence
    std::cout << beast::make_printable(buffer_.data()) << std::endl;
}

//------------------------------------------------------------------------------

int
main(int argc, char** argv)
{
    // Check command line arguments.
    if (argc != 4) {
        std::cerr << "Usage: websocket-client-async-ssl "
                     "<host> <port> <text>\n"
                  << "Example:\n"
                  << "    websocket-client-async-ssl "
                     "echo.websocket.org 443 "
                     "\"Hello, world!\"\n";
        return EXIT_FAILURE;
    }
    auto const host = argv[1];
    auto const port = argv[2];
    auto const text = argv[3];

    // The io_context is required for all I/O
    net::io_context ioc;

    // The SSL context is required, and holds certificates
    ssl::context ctx{ ssl::context::tlsv12_client };

    // This holds the root certificate used for verification
    // load_root_certificates(ctx);

    // Launch the asynchronous operation
    std::make_shared<WSSession>(ioc, ctx)->run(host, port, text);

    // Run the I/O service. The call will return when
    // the socket is closed.
    ioc.run();

    return EXIT_SUCCESS;
}
