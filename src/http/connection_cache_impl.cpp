//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "connection_cache_impl.hpp"

#include "http/connection_impl.hpp"

#include <boost/algorithm/string.hpp>

#include <regex>

namespace http
{
namespace
{
    std::string
    deduce_port(std::string const &scheme, std::string port)
    {
        using boost::algorithm::iequals;

        if (port.empty())
        {
            if (iequals(scheme, "http"))
                port = "http";
            else if (iequals(scheme, "https"))
                port = "https";
            else
                throw system_error(net::error::invalid_argument,
                                   "can't deduce port");
        }

        return port;
    }

    transport_type
    deduce_scheme(std::string const &scheme, std::string const &port)
    {
        using boost::algorithm::iequals;
        if (scheme.empty())
        {
            if (port.empty())
                return transport_type::tcp;

            if (iequals(port, "http") or iequals(port, "80"))
                return transport_type::tcp;

            if (iequals(port, "https") or iequals(port, "443"))
                return transport_type::tls;

            throw system_error(net::error::invalid_argument,
                               "cannot deduce transport");
        }
        else
        {
            if (iequals(scheme, "http"))
                return transport_type::tcp;

            if (iequals(scheme, "https"))
                return transport_type::tls;

            throw system_error(net::error::invalid_argument, "invalid scheme");
        }
    }

    std::tuple< connection_key, std::string >
    parse_url(std::string const &url)
    {
        static const auto url_regex = std::regex(
            R"regex((http|https)://([^/ :]+):?([^/ ]*)((/?[^ #?]*)\x3f?([^ #]*)#?([^ ]*)))regex",
            std::regex_constants::icase | std::regex_constants::optimize);
        auto match = std::smatch();
        if (not std::regex_match(url, match, url_regex))
            throw system_error(net::error::invalid_argument, "invalid url");

        auto &protocol = match[1];
        auto &host     = match[2];
        auto &port_ind = match[3];
        auto &target   = match[4];
        /*
        auto &path     = match[5];
        auto &query    = match[6];
        auto &fragment = match[7];
        */
        return std::make_tuple(
            connection_key { .hostname = host.str(),
                             .port     = deduce_port(protocol, port_ind),
                             .scheme   = deduce_scheme(protocol, port_ind) },
            target.str());
    }
}   // namespace

connection_cache_impl::connection_cache_impl(net::io_strand  exec,
                                             ssl::context   &ssl_ctx,
                                             connect_options options)
: exec_(std::move(exec))
, ssl_ctx_(ssl_ctx)
, options_(options)
, concurrent_requests_available_(get_executor())
{
    concurrent_requests_available_.expires_at(
        net::steady_timer::clock_type::time_point::max());
}

connection_cache_impl::~connection_cache_impl() = default;

net::awaitable< response_type >
connection_cache_impl::call(verb                method,
                            std::string const  &url,
                            std::string         data,
                            beast::http::fields headers,
                            request_options     options)
{
    assert(co_await net::this_coro::executor == exec_);

    auto [key, target] = parse_url(url);

    while (request_count_ >= max_concurrent_requests_)
    {
        error_code ec;
        co_await concurrent_requests_available_.async_wait(
            net::redirect_error(net::use_awaitable, ec));
    }

    ++request_count_;

    auto connection = co_await acquire_connection(key);

    auto request = beast::http::request< beast::http::string_body >(
        method, target, 11, std::move(data), std::move(headers));
    request.set(beast::http::field::host, key.hostname);
    request.prepare_payload();

    auto op = [&]
    { return connection.connection->rest_call(request, options); };

    auto sel = [&](bool cond)
    {
        if (cond)
            return op();
        else
            return net::co_spawn(
                connection.connection->get_executor(), op, net::use_awaitable);
    };

    auto [ec, response] =
        co_await sel(connection.connection->get_executor() == exec_);

    replace_connection(std::move(connection));

    if (--request_count_ < max_concurrent_requests_)
    {
        concurrent_requests_available_.cancel_one();
    }

    if (ec)
        throw system_error(ec);

    co_return std::move(response);
}

auto
connection_cache_impl::acquire_connection(connection_key const &key)
    -> net::awaitable< active_connection >
{
    // find or create the condition variable monitoring the maximum number of
    // connections per host
    auto icond = max_per_host_conditions_.find(key);
    if (icond == max_per_host_conditions_.end())
    {
        icond = max_per_host_conditions_
                    .emplace(std::piecewise_construct,
                             std::tuple(key),
                             std::tuple(get_executor()))
                    .first;
        icond->second.expires_at(net::steady_timer::time_point::max());
    }
    for (;;)
    {
        // get a span of known connections for this endpoint
        auto [first, last] = connection_map_.equal_range(key);

        // search the span for an avaialable connection
        for (; first != last; ++first)
            if (first->second)
                co_return active_connection { .connection =
                                                  std::move(first->second),
                                              .location = first,
                                              .per_host_condition_location =
                                                  icond };

        // if no available connections, see if we can create a new one
        if (static_cast< std::size_t >(std::distance(first, last)) <
            max_connections_per_host_)
        {
            auto candidate =
                std::make_unique< connection_impl >(net::new_strand(exec_),
                                                    ssl_ctx_,
                                                    key.hostname,
                                                    key.port,
                                                    key.scheme,
                                                    options_);
            auto loc =
                connection_map_.emplace_hint(last, key, connection_type());

            co_return active_connection { .connection = std::move(candidate),
                                          .location   = loc,
                                          .per_host_condition_location =
                                              icond };
        }

        // if not possible, then wait for a connection to become available
        co_await icond->second.async_wait(
            asio::experimental::as_single(net::use_awaitable));
    }
}

void
connection_cache_impl::replace_connection(
    connection_cache_impl::active_connection conn)
{
    assert(!conn.location->second.get());

    // replace the connection into the connection map
    conn.location->second = std::move(conn.connection);

    // notify the per-host condition variable that there are connections
    // available for the current host.
    conn.per_host_condition_location->second.cancel_one();
}

const connection_cache_impl::executor_type &
connection_cache_impl::get_executor() const
{
    return exec_;
}

}   // namespace http