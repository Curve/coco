#pragma once

#include "utils.hpp"
#include "../latch/latch.hpp"

#include <exception>
#include <functional>

#include <future>
#include <optional>

#include <type_traits>

namespace coco
{
    namespace impl
    {
#ifdef __cpp_exceptions
        template <typename T, typename F, typename E>
            requires(not std::is_void_v<typename traits<T>::result>)
        coco::stray resolve(T awaitable, F fn, E except)
        try
        {
            std::invoke(fn, co_await std::forward<T>(awaitable));
        }
        catch (...)
        {
            std::invoke(except, std::current_exception());
        }

        template <typename T, typename F, typename E>
            requires(std::is_void_v<typename traits<T>::result>)
        coco::stray resolve(T awaitable, F fn, E except)
        try
        {
            co_await std::forward<T>(awaitable);
            std::invoke(fn);
        }
        catch (...)
        {
            std::invoke(except, std::current_exception());
        }
#else
        template <typename T, typename F>
            requires(not std::is_void_v<typename traits<T>::result>)
        coco::stray resolve(T awaitable, F fn, auto &&...)
        {
            std::invoke(fn, co_await std::forward<T>(awaitable));
        }

        template <typename T, typename F>
            requires(std::is_void_v<typename traits<T>::result>)
        coco::stray resolve(T awaitable, F fn, auto &&...)
        {
            co_await std::forward<T>(awaitable);
            std::invoke(fn);
        }
#endif
    } // namespace impl

    template <awaitable T>
    stray forget(T awaitable)
    {
        co_await std::move(awaitable);
    }

    template <awaitable T>
    auto await(T &&awaitable)
    {
        using result = traits<T>::result;

        auto promise = std::promise<result>{};
        auto fut     = promise.get_future();

        auto resolve = [&promise]<typename... Us>(Us &&...args)
        {
            promise.set_value(std::forward<Us>(args)...);
        };

        auto reject = [&promise]<typename U>(U &&value)
        {
            promise.set_exception(std::forward<U>(value));
        };

        impl::resolve<T>(std::forward<T>(awaitable), resolve, reject);

        return fut.get();
    }

    template <awaitable T, typename Resolve>
    auto then(T awaitable, Resolve resolve)
    {
        then(std::move(awaitable), std::move(resolve), [](auto...) {});
    }

    template <awaitable T, typename Resolve, typename Reject>
    auto then(T awaitable, Resolve resolve, Reject reject)
    {
        impl::resolve(std::move(awaitable), std::move(resolve), std::move(reject));
    }

    template <awaitable... Ts>
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

    template <awaitable T>
    task<std::vector<typename traits<T>::result>> when_all(std::span<T> awaitables)
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
