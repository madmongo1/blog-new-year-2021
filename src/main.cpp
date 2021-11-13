//
// Copyright (c) 2020 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "config.hpp"
#include "http/connection_cache.hpp"
#include "urls_large_data.hpp"

#include <cstdio>
#include <iostream>

net::awaitable< void >
visit_site(http::connection_cache &cache, std::string url)
{
    auto then = std::chrono::steady_clock::now();
    try
    {
        auto result = co_await cache.call(http::verb::get, url);
        auto time   = std::chrono::duration_cast< std::chrono::milliseconds >(
            std::chrono::steady_clock::now() - then);
        std::cout << "GET " << url << " -> " << result->base().result_int()
                  << " " << result->base().reason() << " in " << (time).count()
                  << "ms\n";
    }
    catch (std::exception &e)
    {
        auto time = std::chrono::duration_cast< std::chrono::milliseconds >(
            std::chrono::steady_clock::now() - then);
        std::cout << "GET " << url << " -> "
                  << " exception: " << e.what() << " in " << (time).count()
                  << "ms\n";
    }
}

net::awaitable< void >
crawl()
{
    // build a connection cache
    auto cache = http::connection_cache(co_await net::this_coro::executor);

    // Build the async condition variable in order to know when all requests
    // have completed
    std::size_t pending_count = 0;
    auto        cv = net::steady_timer(co_await net::this_coro::executor);
    cv.expires_at(net::steady_timer::clock_type::time_point::max());

    for (int i = 0; i < 5; ++i)
        for (auto &hostname : urls_large_data())
        {
            auto url = std::string("https://") + hostname + "/";
            ++pending_count;
            net::co_spawn(
                co_await net::this_coro::executor,
                visit_site(cache, std::move(url)),
                [&](std::exception_ptr) {
                    if (--pending_count == 0)
                        cv.cancel_one();
                });
        }

    std::cout << "waiting for " << pending_count << " requests\n";

    while (pending_count)
        co_await cv.async_wait(
            asio::experimental::as_single(net::use_awaitable));

    std::cout << "done\n";
}

int
main()
{
    net::io_context ioctx;

    net::co_spawn(
        ioctx.get_executor(), [] { return crawl(); }, net::detached);

    ioctx.run();
}
