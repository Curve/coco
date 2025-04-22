#pragma once

#include "utils.hpp"
#include "../latch/latch.hpp"

#include <future>
#include <functional>
#include <type_traits>

namespace coco
{
    template <Awaitable T>
    stray forget(T awaitable)
    {
        co_await std::move(awaitable);
    }

    template <Awaitable T>
    auto await(T &&awaitable)
    {
        // NOLINTNEXTLINE(*-coroutine-parameters)
        static auto unpack = []<typename P, typename U>(std::promise<P> promise, U &&awaitable) mutable -> stray
        {
            if constexpr (std::is_void_v<P>)
            {
                co_await std::forward<U>(awaitable);
                promise.set_value();
            }
            else
            {
                promise.set_value(co_await std::forward<U>(awaitable));
            }
        };

        auto promise = std::promise<typename traits<T>::result>{};
        auto fut     = promise.get_future();

        unpack(std::move(promise), std::forward<T>(awaitable));

        return fut.get();
    }

    template <Awaitable T, typename Callback>
    auto then(T awaitable, Callback callback)
    {
        static auto unpack = [](auto awaitable, auto callback) mutable -> stray
        {
            if constexpr (std::is_void_v<typename traits<T>::result>)
            {
                co_await std::move(awaitable);
                std::invoke(callback);
            }
            else
            {
                std::invoke(callback, co_await std::move(awaitable));
            }

            co_return;
        };

        unpack(std::move(awaitable), std::move(callback));
    }

    template <Awaitable... Ts>
    task<std::tuple<typename traits<Ts>::result...>> when_all(Ts... awaitables)
    {
        static constexpr auto N = sizeof...(awaitables);

        auto latch   = coco::latch{N};
        auto results = std::tuple<typename traits<Ts>::result...>{};

        auto spawn = [&]<auto I>(auto awaitable, std::integral_constant<std::size_t, I>) -> stray
        {
            std::get<I>(results) = co_await std::move(awaitable);
            latch.count_down();
        };

        auto tuple = std::forward_as_tuple(awaitables...);

        auto unpack = [&]<auto... Is>(std::integer_sequence<std::size_t, Is...>)
        {
            (spawn(std::move(std::get<Is>(tuple)), std::integral_constant<std::size_t, Is>{}), ...);
        };

        unpack(std::make_index_sequence<N>{});
        co_await latch;

        co_return std::move(results);
    }

    template <Awaitable T>
    task<std::vector<typename traits<T>::result>> when_all(std::vector<T> awaitables)
    {
        auto latch   = coco::latch{static_cast<std::ptrdiff_t>(awaitables.size())};
        auto results = std::vector<typename traits<T>::result>(awaitables.size());

        auto spawn = [&](auto awaitable, auto index) -> stray
        {
            results[index] = co_await std::move(awaitable);
            latch.count_down();
        };

        for (std::size_t i = 0; awaitables.size() > i; ++i)
        {
            spawn(std::move(awaitables[i]), i);
        }
        co_await latch;

        co_return std::move(results);
    }
} // namespace coco
