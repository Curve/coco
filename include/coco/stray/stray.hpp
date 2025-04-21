#pragma once

#include <coroutine>

namespace coco
{
    struct stray
    {
        struct promise_type;
    };

    struct stray::promise_type
    {
        [[nodiscard]] static stray get_return_object();

      public:
        [[nodiscard]] static std::suspend_never initial_suspend();
        [[nodiscard]] static std::suspend_never final_suspend() noexcept;

      public:
        static void return_void();
        static void unhandled_exception();
    };
} // namespace coco

#include "stray.inl"
