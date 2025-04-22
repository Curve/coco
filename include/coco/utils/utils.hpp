#pragma once

#include <vector>

#include "../task/task.hpp"
#include "../stray/stray.hpp"
#include "../traits/traits.hpp"

namespace coco
{
    template <Awaitable T>
    stray forget(T);

    template <Awaitable T>
    auto await(T &&);

    template <Awaitable T, typename Callback>
    auto then(T, Callback);

    template <Awaitable... Ts>
    task<std::tuple<typename traits<Ts>::result...>> when_all(Ts...);

    template <Awaitable T>
    task<std::vector<typename traits<T>::result>> when_all(std::vector<T>);
} // namespace coco

#include "utils.inl"
