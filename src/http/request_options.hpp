//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef NEW_YEAR_2021_HTTP_REQUEST_OPTIONS_HPP_B5647D6CBF7C4B7CB2004F92D1171481
#define NEW_YEAR_2021_HTTP_REQUEST_OPTIONS_HPP_B5647D6CBF7C4B7CB2004F92D1171481

#include <chrono>
#include "async/stop_source.hpp"

namespace http
{
struct request_options
{
    std::chrono::milliseconds write_timeout = std::chrono::seconds(10);
    std::chrono::milliseconds read_timeout  = std::chrono::seconds(10);
    async::stop_token stop;
};

}   // namespace http

#endif   // NEW_YEAR_2021_HTTP_REQUEST_OPTIONS_HPP_B5647D6CBF7C4B7CB2004F92D1171481
