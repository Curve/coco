#pragma once

#include "stray.hpp"

#include <exception>

namespace coco
{
    inline stray stray::promise_type::get_return_object()
    {
        return {};
    }

    inline std::suspend_never stray::promise_type::initial_suspend()
    {
        return {};
    }

    inline std::suspend_never stray::promise_type::final_suspend() noexcept
    {
        return {};
    }

    inline void stray::promise_type::return_void() {}

    inline void stray::promise_type::unhandled_exception()
    {
        std::terminate();
    }
} // namespace coco
