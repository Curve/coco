#pragma once

#include "basic.hpp"

namespace coco
{
    inline basic_task::basic_task(std::future<void> future) : m_future(std::move(future)) {}

    inline void basic_task::get()
    {
        return m_future.get();
    }

    inline basic_task basic_task::promise_type::get_return_object()
    {
        return {m_promise.get_future()};
    }

    inline std::suspend_never basic_task::promise_type::initial_suspend()
    {
        return {};
    }

    inline std::suspend_never basic_task::promise_type::final_suspend() noexcept
    {
        return {};
    }

    inline void basic_task::promise_type::return_void()
    {
        m_promise.set_value();
    }

    inline void basic_task::promise_type::unhandled_exception()
    {
        m_promise.set_exception(std::current_exception());
    }
} // namespace coco
