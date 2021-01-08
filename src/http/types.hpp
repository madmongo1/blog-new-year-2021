//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef NEW_YEAR_2021_HTTP_TYPES_HPP_DFB4491E8B4E48A78F634A2B321AD716
#define NEW_YEAR_2021_HTTP_TYPES_HPP_DFB4491E8B4E48A78F634A2B321AD716

#include "config.hpp"

#include <iostream>
#include <string_view>

namespace http
{
enum class transport_type
{
    tcp,
    tls
};

inline std::string_view
to_string(transport_type b)
{
    switch (b)
    {
    case transport_type::tcp:
        return "tcp";
    case transport_type::tls:
        return "tls";
    }
    return "unknown";
}

inline std::ostream &
operator<<(std::ostream &a, transport_type b)
{
    return a << to_string(b);
}

using tcp_layer = beast::tcp_stream;
using tls_layer = beast::ssl_stream< tcp_layer >;

using verb           = beast::http::verb;
using request_class  = beast::http::request< beast::http::string_body >;
using response_class = beast::http::response< beast::http::string_body >;
using response_type  = std::unique_ptr< response_class >;

}   // namespace http
#endif   // NEW_YEAR_2021_HTTP_TYPES_HPP_DFB4491E8B4E48A78F634A2B321AD716
