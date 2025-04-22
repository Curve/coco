#pragma once

#include <mutex>
#include <vector>
#include <coroutine>

namespace coco
{
    class latch
    {
        class awaiter;

      private:
        std::mutex mutex;
        std::ptrdiff_t counter;
        std::vector<std::coroutine_handle<>> handles;

      public:
        latch(std::ptrdiff_t);

      public:
        void count_down();

      public:
        awaiter operator co_await();
    };

    class latch::awaiter
    {
        friend class latch;

      private:
        latch *m_parent;

      private:
        awaiter(latch *);

      public:
        [[nodiscard]] static bool await_ready();
        bool await_suspend(std::coroutine_handle<>);

      public:
        void await_resume();
    };
} // namespace coco
