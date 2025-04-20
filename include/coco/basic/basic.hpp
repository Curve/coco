#pragma once

#include <future>
#include <coroutine>

namespace coco
{
    struct basic_task
    {
        struct promise_type;

      private:
        std::future<void> m_future;

      public:
        basic_task(std::future<void>);

      public:
        void get();

      public:
        [[nodiscard]] operator const std::future<void> &() &;
    };

    struct basic_task::promise_type
    {
        std::promise<void> m_promise;

      public:
        [[nodiscard]] basic_task get_return_object();

      public:
        [[nodiscard]] static std::suspend_never initial_suspend();
        [[nodiscard]] static std::suspend_never final_suspend() noexcept;

      public:
        void return_void();
        void unhandled_exception();
    };
} // namespace coco

#include "basic.inl"
