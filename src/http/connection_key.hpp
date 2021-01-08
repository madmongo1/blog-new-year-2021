//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef NEW_YEAR_2021_HTTP_CONNECTION_KEY_HPP_1D2C5F73C1CF482A9289DABEEBCBA1C8
#define NEW_YEAR_2021_HTTP_CONNECTION_KEY_HPP_1D2C5F73C1CF482A9289DABEEBCBA1C8

#include "http/types.hpp"

#include <boost/functional/hash.hpp>

#include <functional>
#include <string>

namespace http
{
struct connection_key
{
    std::string    hostname;
    std::string    port;
    transport_type scheme;

    constexpr auto
    as_tie() const
    {
        return std::tie(hostname, port, scheme);
    }
};

std::size_t
hash_value(connection_key const &b);
bool
operator==(connection_key const &a, connection_key const &b);
}   // namespace http

namespace std
{
template <>
struct hash< http::connection_key > : boost::hash< http::connection_key >
{
};
}   // namespace std

#endif   // NEW_YEAR_2021_HTTP_CONNECTION_KEY_HPP_1D2C5F73C1CF482A9289DABEEBCBA1C8
