//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef NEW_YEAR_2021_HTTP_CONNECTION_CACHE_IMPL_HPP_5CE7EC722D674F8AB3649FA5D52B67FB
#define NEW_YEAR_2021_HTTP_CONNECTION_CACHE_IMPL_HPP_5CE7EC722D674F8AB3649FA5D52B67FB

#include "async/stop_source.hpp"
#include "http/connect_options.hpp"
#include "http/connection_key.hpp"
#include "http/request_options.hpp"
#include "http/types.hpp"

#include <memory>
#include <unordered_map>

namespace http
{
struct connection_impl;

struct connection_cache_impl
{
    using executor_type = net::io_strand;

    connection_cache_impl(net::io_strand  exec,
                          ssl::context &  ssl_ctx,
                          connect_options options = {});

    ~connection_cache_impl();

    net::awaitable< response_type >
    call(verb                method,
         std::string const & url,
         std::string         data    = {},
         beast::http::fields headers = {},
         request_options     options = {});

    executor_type const &
    get_executor() const;

  private:
    using connection_type = std::unique_ptr< connection_impl >;
    using connection_map =
        std::unordered_multimap< connection_key, connection_type >;

    using per_host_condition_map =
        std::unordered_map< connection_key, net::steady_timer >;

    struct active_connection
    {
        connection_type                  connection;
        connection_map::iterator         location;
        per_host_condition_map::iterator per_host_condition_location;
    };

    net::awaitable< active_connection >
    acquire_connection(connection_key const &key);

    /// @brief Replace the connection from an active_connection back in the
    /// connection pool and signal any waiters that there is a connection
    /// available.
    /// @param conn is the active_connection to return to the pool
    void
    replace_connection(active_connection conn);

  private:
    net::io_strand  exec_;
    ssl::context &  ssl_ctx_;
    connect_options options_;

    std::size_t max_connections_per_host_ = 2;
    std::unordered_multimap< connection_key,
                             std::unique_ptr< connection_impl > >
        connection_map_;
    std::unordered_map< connection_key, net::steady_timer >
        max_per_host_conditions_;

    std::size_t       max_concurrent_requests_ = 1000;
    std::size_t       request_count_           = 0;
    net::steady_timer concurrent_requests_available_ { exec_ };
};

}   // namespace http

#endif   // NEW_YEAR_2021_HTTP_CONNECTION_CACHE_IMPL_HPP_5CE7EC722D674F8AB3649FA5D52B67FB
