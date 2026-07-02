#pragma once

#include "task.hpp"

#include <memory>
#include <cstdint>

namespace coco
{
    template <typename T>
    struct task<T>::promise_base::index
    {
        enum : std::uint8_t
        {
            none   = 0,
            result = 1,
            error  = 2,
        };

      private:
        template <std::uint8_t>
        static void *make()
        {
            static bool buffer{};
            return std::addressof(buffer);
        }

      public:
        static inline void *running   = static_cast<void *>(nullptr);
        static inline void *completed = make<1>();
        static inline void *abandoned = make<2>();
    };

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

        if (m_handle->is_lazy)
        {
            m_handle.destroy();
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
        using tag      = promise_base::index;
        auto *expected = tag::running;

        if (handle->continuation.compare_exchange_strong(expected, tag::abandoned, std::memory_order_acq_rel))
        {
            return;
        }

        handle.destroy();
    }

    template <typename T>
    void task<T>::promise_base::unhandled_exception()
    {
        value.template emplace<index::error>(std::current_exception());
    }

    inline void task<void>::promise_type::return_void()
    {
        promise_base::value.template emplace<index::result>();
    }

    template <typename T>
    void task<T>::promise_type::return_value(T value)
    {
        promise_base::value.template emplace<promise_base::index::result>(std::move(value));
    }

    template <typename T>
    bool task<T>::promise_base::final_awaiter::await_ready() noexcept
    {
        return false;
    }

    template <typename T>
    std::coroutine_handle<> task<T>::promise_base::final_awaiter::await_suspend(std::coroutine_handle<> handle) noexcept
    {
        using tag = promise_base::index;

        auto *const state = m_handle->continuation.exchange(tag::completed, std::memory_order_acq_rel);
        handle            = std::noop_coroutine();

        if (state == tag::abandoned)
        {
            m_handle.destroy();
        }
        else if (state != tag::running)
        {
            handle = std::coroutine_handle<>::from_address(state);
        }

        return handle;
    }

    template <typename T>
    void task<T>::promise_base::final_awaiter::await_resume() noexcept
    {
    }

    template <typename T>
    bool task<T>::make_lazy::await_ready() noexcept
    {
        return false;
    }

    template <typename T>
    void task<T>::make_lazy::await_suspend(std::coroutine_handle<promise_type> handle) noexcept
    {
        handle.promise().is_lazy = true;
    }

    template <typename T>
    void task<T>::make_lazy::await_resume() noexcept
    {
    }

    template <typename T>
    bool task<T>::awaiter::await_ready() noexcept
    {
        return m_handle->continuation.load(std::memory_order_acquire) == promise_base::index::completed;
    }

    template <typename T>
    std::coroutine_handle<> task<T>::awaiter::await_suspend(std::coroutine_handle<> handle) noexcept
    {
        using tag      = promise_base::index;
        auto *expected = tag::running;

        if (m_handle->continuation.compare_exchange_strong(expected, handle.address(), std::memory_order_acq_rel))
        {
            handle = std::noop_coroutine();
        }

        if (std::exchange(m_handle->is_lazy, false))
        {
            handle = m_handle.get();
        }

        return handle;
    }

    template <typename T>
    T task<T>::awaiter::await_resume()
    {
        if (auto *const exception = std::get_if<promise_base::index::error>(&m_handle->value))
        {
            std::rethrow_exception(*exception);
        }

        if constexpr (!std::is_void_v<T>)
        {
            return std::move(std::get<promise_base::index::result>(m_handle->value));
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
