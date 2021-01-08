//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef NEW_YEAR_2021_CONNECTION_CACHE_HPP
#define NEW_YEAR_2021_CONNECTION_CACHE_HPP

#include "http/connect_options.hpp"
#include "http/default_ssl_context.hpp"
#include "http/request_options.hpp"
#include "http/types.hpp"

#include <memory>

namespace http
{
struct connection_cache_impl;

struct connection_cache
{
    connection_cache(net::any_io_executor const &exec,
                     ssl::context &  ssl_context = default_ssl_context(),
                     connect_options options     = {});
    ~connection_cache();

    net::awaitable< response_type >
    call(beast::http::verb   method,
         std::string const & url,
         std::string         data    = {},
         beast::http::fields headers = {},
         request_options     options = {});

  private:
    std::unique_ptr< connection_cache_impl > impl_;
};

}   // namespace http

#endif   // NEW_YEAR_2021_CONNECTION_CACHE_HPP
