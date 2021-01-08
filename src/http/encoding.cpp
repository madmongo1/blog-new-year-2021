//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "encoding.hpp"

#include <cassert>

namespace http
{
char
hex_digit(int x)
{
    static const char digits[16] = { '0', '1', '2', '3', '4', '5', '6', '7',
                                     '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    assert(x >= 0 && x < 16);
    return digits[x];
}

std::string
urlencode(std::string_view src)
{
    std::string result;
    result.reserve(src.size());

    for (auto c : src)
    {
        const auto i = static_cast< int >(static_cast< unsigned char >(c));
        if (std::isalnum(i) || c == '-' || c == '_' || c == '.' || c == '~')
            result += c;
        else
        {
            auto h = i >> 4;
            auto l = i & 0xf;
            result += '%';
            result += hex_digit(h);
            result += hex_digit(l);
        }
    }
    return result;
}

}   // namespace http
