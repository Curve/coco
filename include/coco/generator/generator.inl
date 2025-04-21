#pragma once

#include "generator.hpp"

#include <algorithm>

namespace coco
{
    template <typename T>
    generator<T>::generator(handle<promise_type> handle) : m_handle(handle)
    {
    }

    template <typename T>
    generator<T>::generator(generator &&other) noexcept = default;

    template <typename T>
    generator<T> &generator<T>::operator=(generator &&other) noexcept = default;

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
    template <typename Pred>
        requires std::same_as<std::invoke_result_t<Pred, T>, bool>
    std::optional<T> generator<T>::find_if(const Pred &pred) &&
    {
        auto it = std::ranges::find_if(*this, pred);

        if (it == end())
        {
            return std::nullopt;
        }

        return std::move(*it);
    }

    template <typename T>
    template <typename U>
        requires std::equality_comparable_with<T, U>
    std::optional<T> generator<T>::find(const U &value) &&
    {
        return std::move(*this).find_if([&](auto &&other) { return value == other; });
    }

    template <typename T>
    template <typename U>
        requires std::equality_comparable_with<T, U>
    std::optional<T> generator<T>::skip(const U &value) &&
    {
        return std::move(*this).find_if([&](auto &&other) { return value != other; });
    }

    template <typename T>
    generator<T>::iterator::iterator() = default;

    template <typename T>
    generator<T>::iterator::iterator(handle<promise_type> handle) : m_handle(handle)
    {
    }

    template <typename T>
    T &generator<T>::iterator::operator*() const
    {
        if (std::holds_alternative<T>(m_handle->value))
        {
            return std::get<T>(m_handle->value);
        }

        std::rethrow_exception(std::get<std::exception_ptr>(m_handle->value));
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
        return {handle<promise_type>::from(this)};
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
    std::suspend_always generator<T>::promise_type::yield_value(U &&val)
    {
        value.template emplace<T>(std::forward<U>(val));
        return {};
    }

    template <typename T>
    void generator<T>::promise_type::return_void()
    {
    }

    template <typename T>
    void generator<T>::promise_type::unhandled_exception()
    {
        value.template emplace<std::exception_ptr>(std::current_exception());
    }
} // namespace coco
