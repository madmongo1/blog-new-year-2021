//
// Copyright (c) 2020 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef DECEMBER_2020_CONDITION_VARIABLE_IMPL_HPP
#define DECEMBER_2020_CONDITION_VARIABLE_IMPL_HPP

#include "config.hpp"

namespace async
{
struct condition_variable_impl
{
    condition_variable_impl(net::any_io_executor exec)
    : timer_(std::move(exec))
    {
        timer_.expires_at(std::chrono::steady_clock::time_point::max());
    }

    template < class Pred >
    net::awaitable< void >
    wait(Pred pred);

    void
    notify_one();

    void
    notify_all();

  private:
    net::steady_timer timer_;
};

template < class Pred >
net::awaitable< void >
condition_variable_impl::wait(Pred pred)
{
    while (!pred())
    {
        error_code ec;
        co_await timer_.async_wait(net::redirect_error(net::use_awaitable, ec));
    }
}

}   // namespace async

#endif   // DECEMBER_2020_CONDITION_VARIABLE_IMPL_HPP
