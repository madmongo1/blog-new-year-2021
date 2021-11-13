//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "http/connection_impl.hpp"

namespace http
{
net::awaitable< error_code >
connection_impl::connect(async::stop_token stop)
{
    error_code ec;
    assert(co_await net::this_coro::executor == exec_);
    if (!stream_.is_open())
        ec = co_await stream_.connect(
            stop, ssl_ctx_, transport_type_, hostname_, port_, options_);
    co_return ec;
}

net::awaitable< std::tuple< error_code, response_type > >
connection_impl::rest_call(request_class const   &request,
                           request_options const &options)
{
    auto response = std::make_unique< response_class >();

    auto ec = error_code(net::error::not_connected);
    if (stream_.is_open())
        ec = co_await rest_call(request, *response, options);

    if (ec && ec != net::error::operation_aborted)
    {
        ec = co_await connect(options.stop);
        if (!ec)
            ec = co_await rest_call(request, *response, options);
    }

    if (ec || response->need_eof())
        stream_.close();

    co_return std::make_tuple(ec, std::move(response));
}

net::awaitable< error_code >
connection_impl::rest_call(request_class const   &request,
                           response_class        &response,
                           request_options const &options)
{
    auto stop = options.stop;
    auto ec   = error_code();
    if (stop.stopped())
        ec = net::error::operation_aborted;

    auto &tcp = stream_.get_tcp_layer();

    if (!ec)
    {
        tcp.expires_after(options.write_timeout);
        auto stopconn = stop.connect([&] { tcp.cancel(); });
        co_await stream_.apply_visitor(
            [&](auto &stream)
            {
                return beast::http::async_write(
                    stream,
                    request,
                    net::redirect_error(net::use_awaitable, ec));
            });
    }

    if (!ec && stop.stopped())
        ec = net::error::operation_aborted;

    if (!ec)
    {
        tcp.expires_after(options.read_timeout);
        auto stopconn = stop.connect([&] { tcp.cancel(); });
        co_await stream_.apply_visitor(
            [&](auto &stream)
            {
                return beast::http::async_read(
                    stream,
                    buf_,
                    response,
                    net::redirect_error(net::use_awaitable, ec));
            });
    }

    if (ec || response.need_eof())
    {
        tcp.socket().shutdown(net::ip::tcp::socket::shutdown_both, ec);
        stream_.close();
    }

    co_return ec;
}

connection_impl::connection_impl(net::io_strand  exec,
                                 ssl::context   &sslctx,
                                 std::string     hostname,
                                 std::string     port,
                                 transport_type  ttype,
                                 connect_options options)
: exec_(std::move(exec))
, ssl_ctx_(sslctx)
, hostname_(std::move(hostname))
, port_(std::move(port))
, transport_type_(ttype)
, options_(options)
, stream_()
, buf_()
{
}

}   // namespace http
