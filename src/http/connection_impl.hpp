//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef PRICER_LIB_POWERTRADE_HTTP_CONNECTION_HPP_0959DA4B9E4549F2A24271794E766515
#define PRICER_LIB_POWERTRADE_HTTP_CONNECTION_HPP_0959DA4B9E4549F2A24271794E766515

#include "http/request_options.hpp"
#include "variant_stream.hpp"

namespace http
{
struct connection_impl
{
    using executor_type = net::io_strand;

    connection_impl(executor_type   exec,
                    ssl::context &  sslctx,
                    std::string     hostname,
                    std::string     port,
                    transport_type  ttype,
                    connect_options options = {});

    executor_type const &
    get_executor()
    {
        return exec_;
    }

    net::awaitable< std::tuple< error_code, response_type > >
    rest_call(request_class const &  request,
              request_options const &options = {});

  private:
    /// @brief Connect to the upstream host if not already connected
    /// @return awaitable containing error code on failure
    net::awaitable< error_code >
    connect(async::stop_token stop);

    net::awaitable< error_code >
    rest_call(request_class const &  request,
              response_class &       response,
              request_options const &options);

    net::io_strand    exec_;
    ssl::context &    ssl_ctx_;
    std::string const hostname_;
    std::string const port_;
    transport_type    transport_type_;
    connect_options   options_;
    variant_stream    stream_;

    beast::flat_buffer buf_;
};

}   // namespace http

#endif   // PRICER_LIB_POWERTRADE_HTTP_CONNECTION_HPP_0959DA4B9E4549F2A24271794E766515
