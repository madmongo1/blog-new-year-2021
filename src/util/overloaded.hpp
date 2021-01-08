//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef NEW_YEAR_2021_OVERLOADED_HPP
#define NEW_YEAR_2021_OVERLOADED_HPP

namespace util
{
// source: https://en.cppreference.com/w/cpp/utility/variant/visit
template < class... Ts >
struct overloaded : Ts...
{
    using Ts::operator()...;
};

template < class... Ts >
overloaded(Ts...) -> overloaded< Ts... >;

}   // namespace util

#endif   // NEW_YEAR_2021_OVERLOADED_HPP
