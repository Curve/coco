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
        static stray get_return_object();

      public:
        static std::suspend_never initial_suspend();
        static std::suspend_never final_suspend() noexcept;

      public:
        static void return_void();
        static void unhandled_exception();
    };
} // namespace coco
