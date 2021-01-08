//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef NEW_YEAR_2021_HTTP_CONNECT_OPTIONS_HPP_E25BF57BB3CF4D89965219D0B37F4C76
#define NEW_YEAR_2021_HTTP_CONNECT_OPTIONS_HPP_E25BF57BB3CF4D89965219D0B37F4C76

#include <chrono>

namespace http
{
struct connect_options
{
    std::chrono::milliseconds resolve_timeout   = std::chrono::seconds(10);
    std::chrono::milliseconds connect_timeout   = std::chrono::seconds(10);
    std::chrono::milliseconds handshake_timeout = std::chrono::seconds(5);
};

}   // namespace http

#endif   // NEW_YEAR_2021_HTTP_CONNECT_OPTIONS_HPP_E25BF57BB3CF4D89965219D0B37F4C76
