#pragma once

#include "task.hpp"

#include <cstdint>

namespace coco
{
    namespace impl
    {
        enum type : std::uint8_t
        {
            none   = 0,
            result = 1,
            error  = 2,
        };
    }

    template <typename T>
    task<T>::task(handle<promise_base> handle) : m_handle(std::move(handle))
    {
    }

    template <typename T>
    task<T>::task() = default;

    template <typename T>
    task<T>::task(task &&) noexcept = default;

    template <typename T>
    task<T> &task<T>::operator=(task &&) noexcept = default;

    template <typename T>
    task<T>::~task()
    {
        if (!m_handle)
        {
            return;
        }

        promise_base::abandon(m_handle);
    }

    template <typename T>
    task<T>::awaiter task<T>::operator co_await() &&
    {
        return awaiter{std::move(m_handle)};
    }

    template <typename T>
    task<T> task<T>::promise_base::get_return_object()
    {
        return {handle<promise_base>::from(this)};
    }

    template <typename T>
    std::suspend_never task<T>::promise_base::initial_suspend()
    {
        return {};
    }

    template <typename T>
    task<T>::promise_base::final_awaiter task<T>::promise_base::final_suspend() noexcept
    {
        return {handle<promise_base>::from(this)};
    }

    template <typename T>
    void task<T>::promise_base::abandon(handle<promise_base> handle)
    {
        handle->has_task.store(false, std::memory_order_release);

        if (!handle.done())
        {
            return;
        }

        handle.destroy();
    }

    template <typename T>
    void task<T>::promise_base::unhandled_exception()
    {
        value.template emplace<impl::type::error>(std::current_exception());
        ready.store(true, std::memory_order_release);
    }

    inline void task<void>::promise_type::return_void()
    {
        promise_base::value.template emplace<impl::type::result>();
        promise_base::ready.store(true, std::memory_order_release);
    }

    template <typename T>
    void task<T>::promise_type::return_value(T value)
    {
        promise_base::value.template emplace<impl::type::result>(std::move(value));
        promise_base::ready.store(true, std::memory_order_release);
    }

    template <typename T>
    bool task<T>::promise_base::final_awaiter::await_ready() noexcept
    {
        return false;
    }

    template <typename T>
    std::coroutine_handle<> task<T>::promise_base::final_awaiter::await_suspend(std::coroutine_handle<> handle) noexcept
    {
        if (auto continuation = m_handle->continuation.exchange(nullptr, std::memory_order_acq_rel))
        {
            handle = continuation;
        }
        else
        {
            handle = std::noop_coroutine();
        }

        if (!m_handle->has_task.load(std::memory_order_acquire))
        {
            m_handle.destroy();
        }

        return handle;
    }

    template <typename T>
    void task<T>::promise_base::final_awaiter::await_resume() noexcept
    {
    }

    template <typename T>
    bool task<T>::wake_on_await::await_ready() noexcept
    {
        return false;
    }

    template <typename T>
    bool task<T>::wake_on_await::await_suspend(std::coroutine_handle<promise_type> handle) noexcept
    {
        auto &promise = handle.promise();
        promise.wake.store(handle, std::memory_order_release);
        return !promise.continuation.load(std::memory_order_acquire);
    }

    template <typename T>
    void task<T>::wake_on_await::await_resume() noexcept
    {
    }

    template <typename T>
    bool task<T>::awaiter::await_ready() noexcept
    {
        return m_handle->ready.load(std::memory_order_acquire);
    }

    template <typename T>
    std::coroutine_handle<> task<T>::awaiter::await_suspend(std::coroutine_handle<> handle) noexcept
    {
        m_handle->continuation.store(handle, std::memory_order_release);

        if (auto wake = m_handle->wake.exchange(nullptr, std::memory_order_acq_rel))
        {
            return wake;
        }

        return std::noop_coroutine();
    }

    template <typename T>
    T task<T>::awaiter::await_resume() noexcept
    {
        if (auto *exception = std::get_if<impl::type::error>(&m_handle->value))
        {
            std::rethrow_exception(*exception);
        }

        if constexpr (!std::is_void_v<T>)
        {
            return std::move(std::get<impl::type::result>(m_handle->value));
        }
    }

    template <typename T>
    task<T>::awaiter::~awaiter()
    {
        if (!m_handle)
        {
            return;
        }

        promise_base::abandon(m_handle);
    }
} // namespace coco
