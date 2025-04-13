#pragma once

#include "generator.hpp"

#include <utility>

namespace coco
{
    template <typename T>
    generator<T>::generator(handle handle) : m_handle(handle)
    {
    }

    template <typename T>
    generator<T>::generator(generator &&other) noexcept : m_handle(std::exchange(other.m_handle, nullptr))
    {
    }

    template <typename T>
    generator<T> &generator<T>::operator=(generator &&other) noexcept
    {
        if (this != &other)
        {
            m_handle = std::exchange(other.m_handle, nullptr);
        }

        return *this;
    }

    template <typename T>
    generator<T>::~generator()
    {
        if (!m_handle)
        {
            return;
        }

        m_handle.destroy();
    }

    template <typename T>
    generator<T>::iterator generator<T>::begin()
    {
        m_handle.resume();
        return {m_handle};
    }

    template <typename T>
    generator<T>::sentinel generator<T>::end() const
    {
        return {};
    }

    template <typename T>
    generator<T>::iterator::iterator() = default;

    template <typename T>
    generator<T>::iterator::iterator(handle handle) : m_handle(handle)
    {
    }

    template <typename T>
    T &generator<T>::iterator::operator*() const
    {
        auto &value = m_handle.promise().m_value;

        if (std::holds_alternative<T>(value))
        {
            return std::get<T>(value);
        }

        std::rethrow_exception(std::get<std::exception_ptr>(value));
    }

    template <typename T>
    generator<T>::iterator &generator<T>::iterator::operator++()
    {
        m_handle.resume();
        return *this;
    }

    template <typename T>
    void generator<T>::iterator::operator++(int) const
    {
        static_cast<void>(operator++());
    }

    template <typename T>
    bool generator<T>::iterator::operator==(const sentinel &) const
    {
        return m_handle.done();
    }

    template <typename T>
    generator<T> generator<T>::promise_type::get_return_object()
    {
        return {handle::from_promise(*this)};
    }

    template <typename T>
    std::suspend_always generator<T>::promise_type::initial_suspend()
    {
        return {};
    }

    template <typename T>
    std::suspend_always generator<T>::promise_type::final_suspend() noexcept
    {
        return {};
    }

    template <typename T>
    template <typename U>
        requires std::constructible_from<T, U>
    std::suspend_always generator<T>::promise_type::yield_value(U &&value)
    {
        m_value.template emplace<T>(std::forward<U>(value));
        return {};
    }

    template <typename T>
    void generator<T>::promise_type::return_void()
    {
    }

    template <typename T>
    void generator<T>::promise_type::unhandled_exception()
    {
        m_value.template emplace<std::exception_ptr>(std::current_exception());
    }
} // namespace coco
