#pragma once

#include "async.hpp"

namespace coco
{
    template <typename T>
    enum async_generator<T>::promise_type::index : std::uint8_t {
        empty = 0,
        last  = 1,
        error = 2,
    };

    template <typename T>
    async_generator<T>::async_generator(handle<promise_type> handle) : m_handle(std::move(handle))
    {
    }

    template <typename T>
    async_generator<T>::async_generator(async_generator &&other) noexcept : m_handle(std::move(other.m_handle))
    {
    }

    template <typename T>
    async_generator<T> &async_generator<T>::operator=(async_generator &&other) noexcept
    {
        if (this != &other)
        {
            m_handle = std::move(other.m_handle);
        }

        return *this;
    }

    template <typename T>
    async_generator<T>::~async_generator()
    {
        if (!m_handle)
        {
            return;
        }

        m_handle.destroy();
    }

    template <typename T>
    async_generator<T>::awaitable<typename async_generator<T>::iterator> async_generator<T>::begin()
    {
        return {m_handle};
    }

    template <typename T>
    async_generator<T>::sentinel async_generator<T>::end() const
    {
        return {};
    }

    template <typename T>
    async_generator<T>::async_generator async_generator<T>::promise_type::get_return_object()
    {
        return {handle<promise_type>::from(this)};
    }

    template <typename T>
    std::suspend_always async_generator<T>::promise_type::initial_suspend()
    {
        return {};
    }

    template <typename T>
    async_generator<T>::promise_type::yield_type async_generator<T>::promise_type::final_suspend() noexcept
    {
        return_void();
        return {handle<promise_type>::from(this)};
    }

    template <typename T>
    template <typename U>
        requires std::constructible_from<T, U>
    async_generator<T>::promise_type::yield_type async_generator<T>::promise_type::yield_value(U &&val)
    {
        value.template emplace<index::last>(std::forward<U>(val));
        return {handle<promise_type>::from(this)};
    }

    template <typename T>
    void async_generator<T>::promise_type::return_void() noexcept
    {
        value.template emplace<index::empty>();
    }

    template <typename T>
    void async_generator<T>::promise_type::unhandled_exception() noexcept
    {
        value.template emplace<index::error>(std::current_exception());
    }

    template <typename T>
    async_generator<T>::iterator::iterator() = default;

    template <typename T>
    async_generator<T>::iterator::iterator(handle<promise_type> handle) : m_handle(std::move(handle))
    {
    }

    template <typename T>
    T &async_generator<T>::iterator::operator*() const
    {
        if (auto *const exception = std::get_if<promise_type::index::error>(&m_handle->value); exception)
        {
            std::rethrow_exception(*exception);
        }

        return std::get<promise_type::index::last>(m_handle->value);
    }

    template <typename T>
    async_generator<T>::awaitable<void> async_generator<T>::iterator::operator++()
    {
        return {m_handle};
    }

    template <typename T>
    bool async_generator<T>::iterator::operator==(const sentinel &) const
    {
        return m_handle->value.index() == promise_type::index::empty;
    }

    template <typename T>
    bool async_generator<T>::promise_type::yield_type::await_ready() noexcept
    {
        return false;
    }

    template <typename T>
    std::coroutine_handle<> async_generator<T>::promise_type::yield_type::await_suspend(std::coroutine_handle<>) noexcept
    {
        return std::exchange(m_handle->waiting, std::noop_coroutine());
    }

    template <typename T>
    void async_generator<T>::promise_type::yield_type::await_resume() noexcept
    {
    }

    template <typename T>
    template <detail::one_of<typename async_generator<T>::iterator, void> U>
    async_generator<T>::awaitable<U>::awaitable(handle<promise_type> handle) : m_handle(std::move(handle))
    {
    }

    template <typename T>
    template <detail::one_of<typename async_generator<T>::iterator, void> U>
    bool async_generator<T>::awaitable<U>::await_ready() noexcept
    {
        return false;
    }

    template <typename T>
    template <detail::one_of<typename async_generator<T>::iterator, void> U>
    std::coroutine_handle<> async_generator<T>::awaitable<U>::await_suspend(std::coroutine_handle<> handle) noexcept
    {
        m_handle->waiting = std::move(handle);
        return m_handle.get();
    }

    template <typename T>
    template <detail::one_of<typename async_generator<T>::iterator, void> U>
    U async_generator<T>::awaitable<U>::await_resume() noexcept
    {
        if constexpr (std::same_as<U, iterator>)
        {
            return {m_handle};
        }
    }
} // namespace coco
