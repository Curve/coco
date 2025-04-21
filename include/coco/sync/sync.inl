#pragma once

#include "sync.hpp"

#include <future>
#include <functional>

namespace coco
{
    template <Awaitable T>
    coco::stray forget(T awaitable)
    {
        co_await std::move(awaitable);
    }

    template <Awaitable T>
    auto await(T &&awaitable)
    {
        // NOLINTNEXTLINE(*-coroutine-parameters)
        static auto unpack = []<typename P, typename U>(std::promise<P> promise, U &&awaitable) mutable -> coco::stray
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
        static auto unpack = [](auto awaitable, auto callback) mutable -> coco::stray
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
} // namespace coco
