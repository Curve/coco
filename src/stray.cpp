#include "stray/stray.hpp"

#include <exception>

namespace coco
{
    stray stray::promise_type::get_return_object()
    {
        return {};
    }

    std::suspend_never stray::promise_type::initial_suspend()
    {
        return {};
    }

    std::suspend_never stray::promise_type::final_suspend() noexcept
    {
        return {};
    }

    void stray::promise_type::return_void() {}

    void stray::promise_type::unhandled_exception()
    {
        std::terminate();
    }
} // namespace coco
