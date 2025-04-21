#pragma once

#include "traits.hpp"

namespace coco
{
    template <typename T>
    struct is_coroutine_handle : std::false_type
    {
    };

    template <typename T>
    struct is_coroutine_handle<std::coroutine_handle<T>> : std::true_type
    {
    };

    template <typename T>
    struct awaiter_of
    {
        using type = void;
    };

    template <typename T>
        requires Awaiter<T>
    struct awaiter_of<T>
    {
        using type = T;
    };

    template <typename T>
        requires Awaiter<decltype(std::declval<T>().operator co_await())>
    struct awaiter_of<T>
    {
        using type = decltype(std::declval<T>().operator co_await());
    };

    template <typename T>
        requires Awaiter<decltype(operator co_await(std::declval<T>()))>
    struct awaiter_of<T>
    {
        using type = decltype(operator co_await(std::declval<T>()));
    };

    template <typename T>
        requires Awaitable<T>
    struct traits<T>
    {
        using awaiter = awaiter_of_t<T>;
        using result  = decltype(std::declval<awaiter>().await_resume());
    };
} // namespace coco
