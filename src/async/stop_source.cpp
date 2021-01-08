//
// Copyright (c) 2020 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "stop_source.hpp"

namespace async
{
namespace detail
{
    std::size_t
    stop_shared_state::connect(std::function< void() > slot)
    {
        if (stopped_)
        {
            slot();
            return 0;
        }
        else
        {
            signals_[++next_key_] = std::move(slot);
            return next_key_;
        }
    }

    void
    stop_shared_state::stop() noexcept
    {
        stopped_  = true;
        auto sigs = std::move(signals_);
        signals_.clear();
        for (auto &e : sigs)
            if (e.second)
                try
                {
                    e.second();
                }
                catch (...)
                {
                }
    }

    void
    stop_shared_state::disconnect(std::size_t id) noexcept
    {
        signals_.erase(id);
    }
}   // namespace detail

stop_source::stop_source()
: impl_(std::make_shared< detail::stop_shared_state >())
{
}

stop_source::stop_source(stop_source &&other) noexcept
: impl_(std::move(other.impl_))
{
}

stop_source &
stop_source::operator=(stop_source &&other) noexcept
{
    auto tmp = stop_source(std::move(other));
    std::swap(impl_, tmp.impl_);
    return *this;
}

stop_source::~stop_source()
{
    stop();
}

stop_token::connection::connection(
    std::shared_ptr< detail::stop_shared_state > impl,
    std::size_t                                  id)
: impl_(std::move(impl))
, id_(id)
{
}

stop_token::connection::~connection()
{
    if (impl_ && id_)
        impl_->disconnect(id_);
}

stop_token::connection &
stop_token::connection::operator=(stop_token::connection &&other) noexcept
{
    if (impl_ && id_)
        impl_->disconnect(id_);

    impl_ = std::move(other.impl_);
    id_   = std::exchange(other.id_, 0);
    return *this;
}
bool
stop_token::stopped() const
{
    return impl_ && impl_->stopped();
}
stop_token::connection
stop_token::connect(std::function< void() > slot)
{
    if (impl_)
        return connection(impl_, impl_->connect(std::move(slot)));
    else
        return connection();
}

stop_token::stop_token() noexcept
: impl_()
{
}

stop_token::stop_token(const stop_source &source)
: impl_(source.impl_)
{
}

}   // namespace async
