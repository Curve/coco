#pragma once

#include <span>
#include <vector>

#include "../task/task.hpp"
#include "../stray/stray.hpp"
#include "../traits/traits.hpp"

namespace coco
{
    template <awaitable T>
    stray forget(T);

    template <awaitable T>
    auto await(T &&);

    template <awaitable T, typename Resolve>
    auto then(T, Resolve);

    template <awaitable T, typename Resolve, typename Reject>
    auto then(T, Resolve, Reject);

    template <awaitable... Ts>
    task<std::tuple<typename traits<Ts>::result...>> when_all(Ts...);

    template <awaitable T>
    task<std::vector<typename traits<T>::result>> when_all(std::span<T>);
} // namespace coco

#include "utils.inl"
