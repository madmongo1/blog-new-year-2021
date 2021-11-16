//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "http/connection_cache.hpp"

#include "http/connection_cache_impl.hpp"

namespace http
{
connection_cache::connection_cache(net::any_io_executor const &exec,
                                   ssl::context &              ssl_context,
                                   connect_options             options)
: impl_(std::make_unique< connection_cache_impl >(
      net::new_strand(net::to_io_executor(exec)),
      ssl_context,
      std::move(options)))
{
}

connection_cache::~connection_cache() = default;

net::awaitable< response_type >
connection_cache::call(beast::http::verb   method,
                       const std::string  &url,
                       std::string         data,
                       beast::http::fields headers,
                       request_options     options)
{
    // DRY - define an operation that performs the inner call.
    auto op = [&]
    {
        return impl_->call(method,
                           url,
                           std::move(data),
                           std::move(headers),
                           std::move(options));
    };

    // deduce the current executor
    auto my_executor = co_await net::this_coro::executor;

    // either call directly or via a spawned coroutine
    if (impl_->get_executor() == my_executor)
        co_return co_await op();
    else
        co_return co_await net::co_spawn(
            impl_->get_executor(), op(), net::use_awaitable);
}

}   // namespace http