#pragma once

#include "utils.hpp"
#include "../latch/latch.hpp"

#include <exception>
#include <functional>

#include <future>
#include <optional>

#include <type_traits>

#ifdef __cpp_exceptions
#define COCO_CATCH(expr, except)                                                                                            \
    try                                                                                                                     \
    {                                                                                                                       \
        expr;                                                                                                               \
    }                                                                                                                       \
    catch (...)                                                                                                             \
    {                                                                                                                       \
        except;                                                                                                             \
    }
#else
#define COCO_CATCH(expr, except) expr;
#endif

namespace coco
{
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

        // NOLINTNEXTLINE(*-reference-coroutine-parameters)
        auto spawn = []<typename U>(U &&awaitable, auto promise) -> coco::stray
        {
            if constexpr (std::is_void_v<result>)
            {
                COCO_CATCH(
                    {
                        co_await std::forward<U>(awaitable);
                        promise.set_value();
                    },
                    promise.set_exception(std::current_exception()));
            }
            else
            {
                COCO_CATCH(promise.set_value(co_await std::forward<U>(awaitable)),
                           promise.set_exception(std::current_exception()));
            }

            co_return;
        };

        spawn(std::forward<T>(awaitable), std::move(promise));

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
        using result = traits<T>::result;

        static auto spawn = [](auto awaitable, auto resolve, [[maybe_unused]] auto reject) -> stray
        {
            if constexpr (std::is_void_v<result>)
            {
                COCO_CATCH(
                    {
                        co_await std::move(awaitable);
                        std::invoke(resolve);
                    },
                    std::invoke(reject, std::current_exception()));
            }
            else
            {
                COCO_CATCH(std::invoke(resolve, co_await std::move(awaitable)),
                           std::invoke(reject, std::current_exception()));
            }
        };

        spawn(std::move(awaitable), std::move(resolve), std::move(reject));
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

#undef COCO_CATCH
