#pragma once

#include "utils.hpp"
#include "../latch/latch.hpp"

#include <future>
#include <optional>
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
        static auto spawn_stray = []<typename P, typename U>(std::promise<P> promise, U &&awaitable) -> stray
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

        spawn_stray(std::move(promise), std::forward<T>(awaitable));

        return fut.get();
    }

    template <Awaitable T, typename Callback>
    auto then(T awaitable, Callback callback)
    {
        static auto spawn_stray = [](auto awaitable, auto callback) -> stray
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

        spawn_stray(std::move(awaitable), std::move(callback));
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

        auto tuple = std::tuple{std::move(awaitables)...};

        auto unpack = [&]<auto... Is>(std::index_sequence<Is...>)
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
        using result = typename traits<T>::result;

        auto latch   = coco::latch{static_cast<std::ptrdiff_t>(awaitables.size())};
        auto results = std::vector<std::optional<result>>(awaitables.size());

        auto spawn = [&](auto awaitable, auto index) -> stray
        {
            results[index].emplace(co_await std::move(awaitable));
            latch.count_down();
        };

        for (std::size_t i = 0; awaitables.size() > i; ++i)
        {
            spawn(std::move(awaitables[i]), i);
        }
        co_await latch;

        auto rtn = std::vector<result>{};
        rtn.reserve(results.size());

        for (auto &result : results)
        {
            rtn.emplace_back(std::move(result.value()));
        }

        co_return std::move(rtn);
    }
} // namespace coco
