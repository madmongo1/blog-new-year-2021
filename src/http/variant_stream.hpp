//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef NEW_YEAR_2021_HTTP_VARIANT_STREAM_HPP_E0C4D4AB638E428BB5AB2E7E67D145DB
#define NEW_YEAR_2021_HTTP_VARIANT_STREAM_HPP_E0C4D4AB638E428BB5AB2E7E67D145DB

#include "async/stop_source.hpp"
#include "config.hpp"
#include "http/connect_options.hpp"
#include "http/types.hpp"
#include "util/overloaded.hpp"

namespace http
{
// clang-format off
template<class F, class X, class Y>
concept same_invoke_result = std::is_same_v
<
    std::invoke_result_t<F, X>,
    std::invoke_result_t<F, Y>
>;

template<class F>
concept mutable_unary_stream_function =
    requires(F const& f, tcp_layer& tcp, tls_layer& tls)
    {
        { f(tcp) };
        { f(tls) };
    }
    &&
    same_invoke_result<F, tcp_layer&, tls_layer&>;

template<class F>
concept const_unary_stream_function =
    requires(F const& f, tcp_layer& tcp, tls_layer& tls)
    {
        { f(tcp) };
        { f(tls) };
    }
    &&
    same_invoke_result<F, tcp_layer&, tls_layer&>;
// clang-format on

struct variant_stream
{
    using variant_type = variant< monostate, tcp_layer, tls_layer >;

    template < mutable_unary_stream_function F >
    auto
    apply_visitor(F &&f) -> std::invoke_result_t< F, tcp_layer & >
    {
        return visit(
            util::overloaded {
                [](monostate &) -> std::invoke_result_t< F, tcp_layer & > {
                    assert(!"logic error - stream not open");
                    throw std::logic_error("stream not open");
                },
                [&](auto &stream) -> std::invoke_result_t< F, tcp_layer & > {
                    return f(stream);
                } },
            var_);
    }

    bool
    is_open() const
    {
        return !holds_alternative< monostate >(var_);
    }

    /// @brief Connect the variant_stream.
    /// @param stop is a stop_token. Stopping the assocaited stop_source will
    /// result in (almost) immediate cancellation of the connect process unless
    /// it has already completed.
    /// @param sslctx is a refernce to the ssl::context to use for this
    /// connection
    /// @param ttype is the type of transport, tcp or tls
    /// @param hostname is the hostname to connect to
    /// @param port is the port/service on which to connect
    /// @param options contains options to guide the connection process
    /// @return An awaitable containing an error_code. If the error_code is
    /// empty, the connection succeeded
    /// @post On success: is_open() == true
    /// @post On error: is_open() == false
    /// @pre is_open == false
    net::awaitable< error_code >
    connect(async::stop_token      stop,
            ssl::context &         sslctx,
            transport_type         ttype,
            std::string const &    hostname,
            std::string const &    port,
            connect_options const &options);

    tcp_layer &
    get_tcp_layer();

    void
    close();

  private:
    variant_type var_;
};

}   // namespace http

#endif   // NEW_YEAR_2021_HTTP_VARIANT_STREAM_HPP_E0C4D4AB638E428BB5AB2E7E67D145DB
