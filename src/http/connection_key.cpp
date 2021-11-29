//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "connection_key.hpp"

#include <ostream>

namespace http
{

std::size_t
hash_value(connection_key const &b)
{
    return boost::hash_value(b.as_tie());
}

bool
operator==(connection_key const &a, connection_key const &b)
{
    return a.as_tie() == b.as_tie();
}

std::ostream &
operator<<(std::ostream &os, connection_key const &k)
{
    return os << k.scheme << "://" << k.hostname << ':' << k.port;
}
}   // namespace http
