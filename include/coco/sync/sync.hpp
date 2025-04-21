#pragma once

#include "../stray/stray.hpp"
#include "../traits/traits.hpp"

namespace coco
{
    template <Awaitable T>
    coco::stray forget(T);

    template <Awaitable T>
    auto await(T &&);

    template <Awaitable T, typename Callback>
    auto then(T, Callback);
} // namespace coco

#include "sync.inl"
