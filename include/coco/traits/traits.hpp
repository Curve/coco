#pragma once

#include <concepts>
#include <coroutine>

namespace coco
{
    template <typename T>
    struct is_coroutine_handle;

    template <typename T>
    static constexpr auto is_coroutine_handle_v = is_coroutine_handle<T>::value;

    template <typename T>
    concept SuspendResult = std::is_void_v<T> || std::same_as<T, bool> || is_coroutine_handle_v<T>;

    template <typename T>
    concept Awaiter = requires(T awaiter, std::coroutine_handle<> handle) {
        { awaiter.await_ready() } -> std::same_as<bool>;
        { awaiter.await_suspend(handle) } -> SuspendResult;
        { awaiter.await_resume() };
    };

    template <typename T>
    struct awaiter_of;

    template <typename T>
    using awaiter_of_t = awaiter_of<T>::type;

    template <typename T>
    concept Awaitable = not std::is_void_v<awaiter_of_t<T>>;

    template <typename T>
    struct traits;
} // namespace coco

#include "traits.inl"
