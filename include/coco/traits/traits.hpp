#pragma once

#include <concepts>
#include <coroutine>

namespace coco
{
    template <typename T>
    struct is_coroutine_handle;

    template <typename T>
    concept coroutine_handle = is_coroutine_handle<T>::value;

    template <typename T>
    concept suspend_result = std::is_void_v<T> || std::same_as<T, bool> || coroutine_handle<T>;

    template <typename T>
    concept awaiter = requires(T awaiter, std::coroutine_handle<> handle) {
        { awaiter.await_ready() } -> std::same_as<bool>;
        { awaiter.await_suspend(handle) } -> suspend_result;
        { awaiter.await_resume() };
    };

    template <typename T>
    struct awaiter_of;

    template <typename T>
    using awaiter_of_t = awaiter_of<T>::type;

    template <typename T>
    concept awaitable = not std::is_void_v<awaiter_of_t<T>>;

    template <typename T>
    struct traits;
} // namespace coco

#include "traits.inl"
