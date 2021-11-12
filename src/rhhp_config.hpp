//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef NEW_YEAR_2021_CONFIG_HPP
#define NEW_YEAR_2021_CONFIG_HPP

#include <boost/asio.hpp>
#include <boost/asio/experimental/as_single.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/variant2/variant.hpp>

using boost::variant2::monostate;
using boost::variant2::variant;

namespace asio  = boost::asio;
namespace ssl   = boost::asio::ssl;
namespace beast = boost::beast;

using boost::system::error_code;
using boost::system::system_error;

namespace net
{
using namespace asio;

using io_executor = io_context::executor_type;

#ifdef MULTI_THREADED

using io_strand = strand< io_executor >;

inline io_strand
new_strand(io_executor const &src)
{
    return net::make_strand(src);
}

inline io_strand
new_strand(io_strand const &src)
{
    return new_strand(src.get_inner_executor());
}

#else

using io_strand = io_context::executor_type;

inline io_strand
new_strand(io_executor const &src)
{
    return src;
}

#endif

inline io_executor
to_io_executor(any_io_executor const &src)
{
    if (auto e = src.target< io_context::executor_type >())
    {
        return *e;
    }
    else if (auto s = src.target< strand< io_context::executor_type > >())
    {
        return s->get_inner_executor();
    }
    else
    {
        assert(!"unknown executor type");
        std::abort();
    }
}
}   // namespace net
#endif   // NEW_YEAR_2021_CONFIG_HPP
