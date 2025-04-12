#pragma once

#include "../basic/basic.hpp"

#include <future>
#include <coroutine>

namespace coco
{
    template <typename T>
    class [[nodiscard]] task
    {
        struct state;
        struct awaiter;
        struct promise_base;

      public:
        struct promise_type;

      private:
        std::future<T> m_future;
        std::shared_ptr<state> m_state;

      public:
        task(task &&) noexcept;
        task(std::future<T>, std::shared_ptr<state>);

      public:
        [[nodiscard]] T get();

      public:
        template <typename Callback>
        basic_task then(Callback) &&;

      public:
        [[nodiscard]] awaiter operator co_await() &&;
    };

    template <typename T>
    struct task<T>::promise_base
    {
        struct final_awaiter;

      public:
        std::promise<T> m_promise;
        std::shared_ptr<state> m_state;

      public:
        [[nodiscard]] task<T> get_return_object();

      public:
        [[nodiscard]] std::suspend_never initial_suspend();
        [[nodiscard]] final_awaiter final_suspend() noexcept;

      public:
        void unhandled_exception();
    };

    template <>
    struct task<void>::promise_type : task<void>::promise_base
    {
        void return_void();
    };

    template <typename T>
    struct task<T>::promise_type : task<T>::promise_base
    {
        void return_value(T);
    };

    template <typename T>
    struct task<T>::promise_base::final_awaiter
    {
        std::coroutine_handle<> m_handle;
        std::shared_ptr<state> m_state;

      public:
        [[nodiscard]] bool await_ready() noexcept;
        [[nodiscard]] std::coroutine_handle<> await_suspend(std::coroutine_handle<>) noexcept;

      public:
        void await_resume() noexcept;
    };

    template <typename T>
    struct task<T>::awaiter
    {
        std::future<T> m_future;
        std::shared_ptr<state> m_state;

      public:
        [[nodiscard]] bool await_ready() noexcept;
        [[nodiscard]] bool await_suspend(std::coroutine_handle<>) noexcept;

      public:
        [[nodiscard]] T await_resume() noexcept;
    };
} // namespace coco

#include "task.inl"
