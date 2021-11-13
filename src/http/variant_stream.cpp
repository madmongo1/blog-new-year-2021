//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "variant_stream.hpp"

#include "util/overloaded.hpp"

namespace http
{
namespace
{
    net::awaitable<
        std::tuple< error_code, net::ip::tcp::resolver ::results_type > >
    resolve(async::stop_token         stop,
            std::string const        &hostname,
            std::string const        &port,
            std::chrono::milliseconds timeout)
    {
        error_code                            error;
        net::ip::tcp::resolver ::results_type results;

        if (stop.stopped())
        {
            error = net::error::operation_aborted;
        }
        else
        {
            auto exec = co_await net::this_coro::executor;

            auto resolver = net::ip::tcp::resolver(exec);
            auto timer    = net::steady_timer(exec);
            timer.expires_after(timeout);

            // Use the timer cv as an asynchronous condition variable.
            auto cv = net::steady_timer(exec);
            cv.expires_at(net::steady_timer::clock_type::time_point::max());
            bool pending_timer   = true;
            bool pending_resolve = true;
            resolver.async_resolve(
                hostname,
                port,
                [&](error_code ec, net::ip::tcp::resolver::results_type rslts)
                {
                    timer.cancel();
                    if (pending_timer)
                    {
                        error   = ec;
                        results = std::move(rslts);
                    }
                    pending_resolve = false;
                    cv.cancel_one();
                });
            timer.async_wait(
                [&](error_code)
                {
                    resolver.cancel();
                    if (pending_resolve)
                    {
                        error = net::error::timed_out;
                    }
                    pending_timer = false;
                    cv.cancel_one();
                });

            auto stopconn = stop.connect(
                [&]
                {
                    error = net::error::operation_aborted;
                    timer.cancel();
                    resolver.cancel();
                });

            while (pending_timer || pending_resolve)
            {
                error_code ec;
                co_await cv.async_wait(
                    asio::redirect_error(net::use_awaitable, ec));
            }
        }
        co_return std::make_tuple(error, std::move(results));
    }

    net::awaitable< error_code >
    connect_tcp(async::stop_token                    stop,
                tcp_layer                           &tcp,
                net::ip::tcp::resolver::results_type results,
                std::chrono::milliseconds            timeout)
    {
        error_code ec;
        if (stop.stopped())
        {
            ec = net::error::operation_aborted;
        }
        else
        {
            tcp.expires_after(timeout);
            auto stopconn = stop.connect([&] { tcp.cancel(); });
            auto endpoint = co_await tcp.async_connect(
                results, net::redirect_error(net::use_awaitable, ec));
            boost::ignore_unused(endpoint);
        }
        co_return ec;
    }

    net::awaitable< error_code >
    handshake(async::stop_token         stop,
              tls_layer                &tls,
              std::string const        &hostname,
              std::chrono::milliseconds timeout)
    {
        error_code ec;
        if (stop.stopped())
        {
            ec = net::error::operation_aborted;
        }
        else
        {
            if (SSL_set_tlsext_host_name(tls.native_handle(), hostname.c_str()))
            {
                auto stopconn =
                    stop.connect([&] { tls.next_layer().cancel(); });
                tls.next_layer().expires_after(timeout);
                co_await tls.async_handshake(
                    ssl::stream_base::client,
                    net::redirect_error(net::use_awaitable, ec));
            }
            else
                ec.assign(static_cast< int >(::ERR_get_error()),
                          net::error::get_ssl_category());
        }
        co_return ec;
    }
}   // namespace

net::awaitable< error_code >
variant_stream::connect(async::stop_token      stop,
                        ssl::context          &sslctx,
                        transport_type         ttype,
                        std::string const     &hostname,
                        std::string const     &port,
                        connect_options const &options)
{
    assert(!is_open());

    if (stop.stopped())
        co_return net::error::operation_aborted;

    auto [ec, results] =
        co_await resolve(stop, hostname, port, options.resolve_timeout);
    if (!ec)
    {
        switch (ttype)
        {
        case transport_type::tcp:
            ec = co_await connect_tcp(
                stop,
                var_.emplace< tcp_layer >(co_await net::this_coro::executor),
                results,
                options.connect_timeout);
            break;
        case transport_type::tls:
        {
            auto &tls = var_.emplace< tls_layer >(
                co_await net::this_coro::executor, sslctx);
            ec = co_await connect_tcp(
                stop, tls.next_layer(), results, options.connect_timeout);
            if (!ec)
                ec = co_await handshake(
                    stop, tls, hostname, options.handshake_timeout);
            break;
        }
        }
    }

    if (ec)
        var_.emplace< monostate >();
    co_return ec;
}

tcp_layer &
variant_stream::get_tcp_layer()
{
    return apply_visitor(util::overloaded {
        [](tcp_layer &layer) -> tcp_layer & { return layer; },
        [](tls_layer &layer) -> tcp_layer & { return layer.next_layer(); } });
}

void
variant_stream::close()
{
    if (is_open())
    {
        auto      &tcp = get_tcp_layer();
        error_code ec;
        tcp.socket().shutdown(tcp_layer::socket_type::shutdown_both, ec);
        tcp.close();
        var_.emplace< monostate >();
    }
}

}   // namespace http
