#pragma once

#include <thread>
#include <coroutine>

namespace coco::tests
{
    struct co_sleep
    {
        std::chrono::milliseconds timeout;

      public:
        static constexpr bool await_ready()
        {
            return false;
        }

        void await_suspend(std::coroutine_handle<> handle) const
        {
            std::thread thread{[handle, timeout = timeout]()
                               {
                                   std::this_thread::sleep_for(timeout);
                                   handle.resume();
                               }};

            thread.detach();
        }

        void await_resume() {}
    };

} // namespace coco::tests
