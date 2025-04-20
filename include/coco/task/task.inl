#pragma once

#include "task.hpp"

#include <utility>
#include <functional>

namespace coco
{
    template <typename T>
    struct task<T>::state
    {
        std::mutex mutex;

      public:
        std::coroutine_handle<> idle;
        std::coroutine_handle<> continuation;
    };

    template <typename T>
    task<T>::task(std::future<T> future, std::shared_ptr<state> state)
        : m_future(std::move(future)), m_state(std::move(state))
    {
    }

    template <typename T>
    task<T>::task() = default;

    template <typename T>
    task<T>::task(task &&) noexcept = default;

    template <typename T>
    task<T> &task<T>::operator=(task &&) noexcept = default;

    template <typename T>
    T task<T>::get()
    {
        return m_future.get();
    }

    template <typename T>
    template <typename Callback>
    basic_task task<T>::then(Callback callback) &&
    {
        auto self = std::move(*this);

        if constexpr (std::is_void_v<T>)
        {
            co_await std::move(self);
            std::invoke(callback);
        }
        else
        {
            std::invoke(callback, co_await std::move(self));
        }
    }

    template <typename T>
    task<T>::awaiter task<T>::operator co_await() &&
    {
        return awaiter{std::move(m_future), std::move(m_state)};
    }

    template <typename T>
    task<T> task<T>::promise_base::get_return_object()
    {
        m_state = std::make_shared<state>();
        return {m_promise.get_future(), m_state};
    }

    template <typename T>
    std::suspend_never task<T>::promise_base::initial_suspend()
    {
        return {};
    }

    template <typename T>
    task<T>::promise_base::final_awaiter task<T>::promise_base::final_suspend() noexcept
    {
        return {std::coroutine_handle<promise_base>::from_promise(*this), std::move(m_state)};
    }

    template <typename T>
    void task<T>::promise_base::unhandled_exception()
    {
        m_promise.set_exception(std::current_exception());
    }

    inline void task<void>::promise_type::return_void()
    {
        promise_base::m_promise.set_value();
    }

    template <typename T>
    void task<T>::promise_type::return_value(T value)
    {
        promise_base::m_promise.set_value(std::move(value));
    }

    template <typename T>
    bool task<T>::promise_base::final_awaiter::await_ready() noexcept
    {
        return false;
    }

    template <typename T>
    std::coroutine_handle<> task<T>::promise_base::final_awaiter::await_suspend(std::coroutine_handle<> handle) noexcept
    {
        auto lock = std::lock_guard{m_state->mutex};
        handle    = m_state->continuation ? m_state->continuation : std::noop_coroutine();

        // Terrible hack above - MSVC exhibits buggy behavior as it stores the local variables of this function inside of
        // the coroutine frame instead of the stack (this is not allowed). Thus calling `m_handle.destroy()` will invalidate
        // all local variables here and lead to a segmentation fault. However, the handle passed to this function is always
        // properly stored on the stack, thus we save the coroutine we want to do a symmetric transfer to in the handle, thus
        // avoiding the issue.
        //
        // Relevant issues:
        // https://developercommunity.visualstudio.com/t/Incorrect-code-generation-for-symmetric/1659260?scope=follow,
        // https://developercommunity.visualstudio.com/t/Incorrect-codegen-in-await_suspend-aroun/10454102

        m_handle.destroy();

        return handle;
    }

    template <typename T>
    void task<T>::promise_base::final_awaiter::await_resume() noexcept
    {
    }

    template <typename T>
    bool task<T>::awaiter::await_ready() noexcept
    {
        return m_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
    }

    template <typename T>
    std::coroutine_handle<> task<T>::awaiter::await_suspend(std::coroutine_handle<> handle) noexcept
    {
        auto lock             = std::lock_guard{m_state->mutex};
        m_state->continuation = handle;

        if (m_state->idle)
        {
            return m_state->idle;
        }

        if (await_ready())
        {
            return handle;
        }

        return std::noop_coroutine();
    }

    template <typename T>
    T task<T>::awaiter::await_resume() noexcept
    {
        return m_future.get();
    }

    template <typename T>
    bool task<T>::idle::await_ready() noexcept
    {
        return false;
    }

    template <typename T>
    void task<T>::idle::await_suspend(std::coroutine_handle<promise_type> handle) noexcept
    {
        auto state  = handle.promise().m_state;
        auto guard  = std::lock_guard{state->mutex};
        state->idle = handle;
    }

    template <typename T>
    void task<T>::idle::await_resume() noexcept
    {
    }
} // namespace coco
