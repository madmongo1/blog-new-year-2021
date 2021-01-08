//
// Copyright (c) 2020 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "condition_variable_impl.hpp"

namespace async
{
void
condition_variable_impl::notify_one()
{
    timer_.cancel_one();
}

void
condition_variable_impl::notify_all()
{
    timer_.cancel();
}

}   // namespace async