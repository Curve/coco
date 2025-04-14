#pragma once

#include "promise.hpp"

#include <utility>
#include <functional>

namespace coco
{
    template <typename T>
    struct promise_base<T>::state
    {
        std::mutex mutex;
        std::coroutine_handle<> handle;
    };

    template <typename T>
    promise_base<T>::promise_base() : m_state(std::make_shared<state>())
    {
    }

    template <typename T>
    promise_base<T>::promise_base(promise_base &&) noexcept = default;

    template <typename T>
    promise_base<T> &promise_base<T>::operator=(promise_base &&) noexcept = default;

    template <typename T>
    promise_base<T>::~promise_base()
    {
        if (!m_state)
        {
            return;
        }

        std::lock_guard guard{m_state->mutex};

        if (!m_state->handle)
        {
            return;
        }

        m_state->handle.destroy();
    }

    template <typename T>
    void promise_base<T>::resume()
    {
        std::lock_guard guard{m_state->mutex};

        if (!m_state->handle)
        {
            return;
        }

        m_state->handle.resume();
        m_state->handle = {};
    }

    template <typename T>
    future<T> promise_base<T>::get_future()
    {
        return future<T>{m_promise.get_future(), m_state};
    }

    template <typename T>
    void promise_base<T>::set_exception(std::exception_ptr exception)
    {
        m_promise.set_exception(exception);
        resume();
    }

    template <typename T>
    void promise<T>::set_value(T value)
    {
        promise_base<T>::m_promise.set_value(std::move(value));
        promise_base<T>::resume();
    }

    inline void promise<void>::set_value()
    {
        promise_base<void>::m_promise.set_value();
        promise_base<void>::resume();
    }

    template <typename T>
    future<T>::future(std::future<T> future, std::shared_ptr<state> state)
        : m_future(std::move(future)), m_state(std::move(state))
    {
    }

    template <typename T>
    future<T>::future() = default;

    template <typename T>
    future<T>::future(future &&) noexcept = default;

    template <typename T>
    future<T> &future<T>::operator=(future &&) noexcept = default;

    template <typename T>
    T future<T>::get()
    {
        return m_future.get();
    }

    template <typename T>
    template <typename Callback>
    basic_task future<T>::then(Callback callback) &&
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
    future<T>::awaiter future<T>::operator co_await() &&
    {
        return {std::move(m_future), std::move(m_state)};
    }

    template <typename T>
    bool future<T>::awaiter::await_ready() const noexcept
    {
        return m_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
    }

    template <typename T>
    bool future<T>::awaiter::await_suspend(std::coroutine_handle<> handle) noexcept
    {
        auto lock       = std::lock_guard{m_state->mutex};
        m_state->handle = handle;

        return !await_ready();
    }

    template <typename T>
    T future<T>::awaiter::await_resume()
    {
        return m_future.get();
    }
} // namespace coco
