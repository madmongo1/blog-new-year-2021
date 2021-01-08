//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "default_ssl_context.hpp"

namespace http
{
struct init
{
    init()
    : context(ssl::context_base::tlsv12_client)
    {
        /// @todo initialise certificate store here
    }

    ssl::context context;
};

ssl::context &
default_ssl_context()
{
    static init x;
    return x.context;
};

}   // namespace http
