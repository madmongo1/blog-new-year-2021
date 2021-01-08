//
// Copyright (c) 2020 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef DECEMBER_2020_STOP_SOURCE_HPP
#define DECEMBER_2020_STOP_SOURCE_HPP

#include <functional>
#include <unordered_map>

namespace async
{
namespace detail
{
    struct stop_shared_state
    {
        std::size_t
        connect(std::function< void() > slot);

        void
        disconnect(std::size_t id) noexcept;

        bool
        stopped() const
        {
            return stopped_;
        }

        void
        stop() noexcept;

      private:
        std::unordered_map< std::size_t, std::function< void() > > signals_;
        std::size_t next_key_ = 0;
        bool        stopped_  = false;
    };
}   // namespace detail

struct stop_token;

struct stop_source
{
    stop_source();
    stop_source(stop_source &&other) noexcept;
    stop_source(stop_source const &other) = delete;
    stop_source &
    operator=(stop_source &&other) noexcept;
    stop_source &
    operator=(stop_source const &other) = delete;
    ~stop_source();

    void
    stop() noexcept
    {
        if (impl_)
            impl_->stop();
    }

  private:
    std::shared_ptr< detail::stop_shared_state > impl_;
    friend stop_token;
};

struct stop_token
{
    struct connection
    {
        connection()                            = default;
        connection(connection &&other) noexcept = default;
        connection &
        operator=(connection &&other) noexcept;
        ~connection();

      private:
        connection(std::shared_ptr< detail::stop_shared_state > impl,
                   std::size_t                                  id);

        std::shared_ptr< detail::stop_shared_state > impl_ = nullptr;
        std::size_t                                  id_   = 0;

        friend stop_token;
    };

    stop_token(stop_source const &source);

    stop_token() noexcept;

    [[nodiscard]] bool
    stopped() const;

    connection
    connect(std::function< void() > slot);

  private:
    std::shared_ptr< detail::stop_shared_state > impl_;
};

}   // namespace async

#endif   // DECEMBER_2020_STOP_SOURCE_HPP
