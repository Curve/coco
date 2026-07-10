#pragma once

#include "task.hpp"

#include <memory>
#include <cstdint>

namespace coco
{
    namespace detail::of_task::index
    {
        enum : std::uint8_t
        {
            none   = 0,
            result = 1,
            error  = 2,
        };
    } // namespace detail::of_task::index

    namespace detail::of_task::tag
    {
        template <std::uint8_t>
        void *make()
        {
            [[maybe_unused]] static std::uint8_t buffer{};
            return std::addressof(buffer);
        }

        template <>
        inline void *make<0>()
        {
            return nullptr;
        }

        inline void *running   = make<0>();
        inline void *completed = make<1>();
        inline void *abandoned = make<2>();
    } // namespace detail::of_task::tag

    template <typename T, task_options opts>
    task<T, opts>::task(handle<promise_base> handle) : m_handle(std::move(handle))
    {
    }

    template <typename T, task_options opts>
    task<T, opts>::task() = default;

    template <typename T, task_options opts>
    task<T, opts>::task(task &&) noexcept = default;

    template <typename T, task_options opts>
    task<T, opts> &task<T, opts>::operator=(task &&) noexcept = default;

    template <typename T, task_options opts>
    task<T, opts>::~task()
    {
        if (!m_handle)
        {
            return;
        }

        if constexpr (opts.lazy)
        {
            m_handle.destroy();
            return;
        }

        promise_base::abandon(m_handle);
    }

    template <typename T, task_options opts>
    task<T, opts>::awaiter task<T, opts>::operator co_await() &&
    {
        return awaiter{std::move(m_handle)};
    }

    template <typename T, task_options opts>
    task<T, opts> task<T, opts>::promise_base::get_return_object()
    {
        return {handle<promise_base>::from(this)};
    }

    template <typename T, task_options opts>
    auto task<T, opts>::promise_base::initial_suspend()
    {
        if constexpr (opts.lazy)
        {
            return std::suspend_always{};
        }
        else
        {
            return std::suspend_never{};
        }
    }

    template <typename T, task_options opts>
    task<T, opts>::promise_base::final_awaiter task<T, opts>::promise_base::final_suspend() noexcept
    {
        return {handle<promise_base>::from(this)};
    }

    template <typename T, task_options opts>
    void task<T, opts>::promise_base::abandon(handle<promise_base> handle)
    {
        namespace tag  = detail::of_task::tag;
        auto *expected = tag::running;

        if (handle->continuation.compare_exchange_strong(expected, tag::abandoned, std::memory_order_acq_rel))
        {
            return;
        }

        handle.destroy();
    }

    template <typename T, task_options opts>
    void task<T, opts>::promise_base::unhandled_exception()
    {
        value.template emplace<detail::of_task::index::error>(std::current_exception());
    }

    template <task_options opts>
    void detail::of_task::promise_type<void, opts>::return_void()
    {
        task<void, opts>::promise_base::value.template emplace<detail::of_task::index::result>();
    }

    template <typename T, task_options opts>
    void detail::of_task::promise_type<T, opts>::return_value(T value)
    {
        task<T, opts>::promise_base::value.template emplace<detail::of_task::index::result>(std::move(value));
    }

    template <typename T, task_options opts>
    bool task<T, opts>::promise_base::final_awaiter::await_ready() noexcept
    {
        return false;
    }

    template <typename T, task_options opt>
    std::coroutine_handle<> task<T, opt>::promise_base::final_awaiter::await_suspend(std::coroutine_handle<> handle) noexcept
    {
        namespace tag = detail::of_task::tag;

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

    template <typename T, task_options opts>
    void task<T, opts>::promise_base::final_awaiter::await_resume() noexcept
    {
    }

    template <typename T, task_options opts>
    bool task<T, opts>::awaiter::await_ready() noexcept
    {
        return m_handle->continuation.load(std::memory_order_acquire) == detail::of_task::tag::completed;
    }

    template <typename T, task_options opts>
    std::coroutine_handle<> task<T, opts>::awaiter::await_suspend(std::coroutine_handle<> handle) noexcept
    {
        auto *expected = detail::of_task::tag::running;

        if (m_handle->continuation.compare_exchange_strong(expected, handle.address(), std::memory_order_acq_rel))
        {
            handle = std::noop_coroutine();
        }

        if constexpr (opts.lazy)
        {
            handle = m_handle.get();
        }

        return handle;
    }

    template <typename T, task_options opts>
    T task<T, opts>::awaiter::await_resume()
    {
        if (auto *const exception = std::get_if<detail::of_task::index::error>(&m_handle->value))
        {
            std::rethrow_exception(*exception);
        }

        if constexpr (!std::is_void_v<T>)
        {
            return std::move(std::get<detail::of_task::index::result>(m_handle->value));
        }
    }

    template <typename T, task_options opts>
    task<T, opts>::awaiter::~awaiter()
    {
        if (!m_handle)
        {
            return;
        }

        promise_base::abandon(m_handle);
    }
} // namespace coco
