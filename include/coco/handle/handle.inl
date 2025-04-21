#pragma once

#include "handle.hpp"

#include <utility>

namespace coco
{
    template <typename T>
    handle<T>::handle() : m_handle(nullptr)
    {
    }

    template <typename T>
    handle<T>::handle(handle_t handle) : m_handle(handle)
    {
    }

    template <typename T>
    handle<T>::handle(const handle &other) = default;

    template <typename T>
    handle<T>::handle(handle &&other) noexcept : m_handle(std::exchange(other.m_handle, nullptr))
    {
    }

    template <typename T>
    handle<T> &handle<T>::operator=(const handle &) = default;

    template <typename T>
    handle<T> &handle<T>::operator=(handle &&other) noexcept
    {
        if (this != &other)
        {
            m_handle = std::exchange(other.m_handle, nullptr);
        }

        return *this;
    }

    template <typename T>
    bool handle<T>::done() const
    {
        return m_handle.done();
    }

    template <typename T>
    handle<T>::handle_t handle<T>::get() const
    {
        return m_handle;
    }

    template <typename T>
    void handle<T>::resume() const
    {
        m_handle.resume();
    }

    template <typename T>
    void handle<T>::destroy() const
    {
        m_handle.destroy();
    }

    template <typename T>
    auto *handle<T>::operator->() const
    {
        return std::addressof(m_handle.promise());
    }

    template <typename T>
    handle<T>::operator bool() const
    {
        return static_cast<bool>(m_handle);
    }

    template <typename T>
    handle<T> handle<T>::from(T *promise)
    {
        return handle_t::from_promise(*promise);
    }
} // namespace coco
