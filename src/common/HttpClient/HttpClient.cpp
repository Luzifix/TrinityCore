#include "HttpClient.h"
#include "Log.h"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <cstdlib>
#include <iostream>

namespace beast = boost::beast; 
namespace http = beast::http;   
namespace net = boost::asio;    
using tcp = net::ip::tcp;

/*static*/ std::string Trinity::HttpClient::HttpGet(const char* host, const char* path, const char* port /*= "80"*/)
{
    try
    {
        // The io_context is required for all I/O
        net::io_context ioc;

        // These objects perform our I/O
        tcp::resolver resolver(ioc);
        beast::tcp_stream stream(ioc);

        // Look up the domain name
        auto const results = resolver.resolve(host, port);

        // Make the connection on the IP address we get from a lookup
        stream.connect(results);

        // Set up an HTTP GET request message
        int version = 11;
        http::request<http::string_body> req{ http::verb::get, path, version };
        req.set(http::field::host, host);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        // Send the HTTP request to the remote host
        http::write(stream, req);

        // This buffer is used for reading and must be persisted
        beast::flat_buffer buffer;

        // Declare a container to hold the response
        http::response<http::string_body> res;

        // Receive the HTTP response
        http::read(stream, buffer, res);

        // Write the message to standard out
        std::string responseBody = res.body().data();

        // Gracefully close the socket
        beast::error_code ec;
        stream.socket().shutdown(tcp::socket::shutdown_both, ec);

        // not_connected happens sometimes
        // so don't bother reporting it.
        //
        if (ec && ec != beast::errc::not_connected)
            throw beast::system_error{ ec };

        return responseBody;
    }
    catch (std::exception const& e)
    {
        TC_LOG_ERROR("server", "Cant fetch %s%s! Error: %s", host, path, e.what());

        return "";
    }
}

/*static*/ std::string Trinity::HttpClient::HttpsPostJson(const char* host, const char* path, JSON jsonBody, const char* port /*= "443"*/)
{
    using namespace boost::beast;
    using namespace boost::asio;

    try
    {
        // The io_context is required for all I/O
        net::io_context ioc;

        ssl::context ctx(ssl::context::sslv23_client);
        ssl::stream<ip::tcp::socket> ssocket = { ioc, ctx };
        ip::tcp::resolver resolver(ioc);
        auto it = resolver.resolve(host, port);
        connect(ssocket.lowest_layer(), it);
        ssocket.handshake(ssl::stream_base::handshake_type::client);

        // Set up an HTTP GET request message
        int version = 11;
        http::request<http::string_body> req{ http::verb::post, path, version };
        req.set(http::field::host, host);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        req.set(http::field::content_type, "application/json");
        req.body() = jsonBody.dump();
        req.prepare_payload();

        req.set(http::field::host, host);
        http::write(ssocket, req);
        http::response<http::string_body> res;
        flat_buffer buffer;
        http::read(ssocket, buffer, res);

        // Write the message to standard out
        std::string responseBody = res.body().data();

        // Gracefully close the socket
        ssocket.lowest_layer().close();

        return responseBody;
    }
    catch (std::exception const& e)
    {
        TC_LOG_ERROR("server", "Cant post %s%s! Error: %s", host, path, e.what());

        return "";
    }
}
