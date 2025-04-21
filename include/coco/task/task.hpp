#pragma once

#include "../handle/handle.hpp"

#include <cstdint>
#include <exception>

#include <atomic>
#include <variant>

namespace coco
{
    namespace impl
    {
        enum type : std::uint8_t
        {
            none   = 0,
            result = 1,
            error  = 2,
        };

        struct empty
        {
        };
    } // namespace impl

    template <typename T>
    class task
    {
        struct awaiter;
        struct promise_base;

      public:
        struct promise_type;
        struct wake_on_await;

      private:
        handle<promise_base> m_handle;

      private:
        task(handle<promise_base>);

      public:
        task();

      public:
        task(task &&) noexcept;
        task &operator=(task &&) noexcept;

      public:
        ~task();

      public:
        [[nodiscard]] awaiter operator co_await();
    };

    template <typename T>
    struct task<T>::promise_base
    {
        struct final_awaiter;

      public:
        using result = std::conditional_t<std::is_void_v<T>, impl::empty, T>;

      public:
        std::atomic<bool> has_task{true};

      public:
        std::atomic<bool> ready;
        std::variant<std::monostate, result, std::exception_ptr> value;

      public:
        std::atomic<std::coroutine_handle<>> wake;
        std::atomic<std::coroutine_handle<>> continuation;

      public:
        [[nodiscard]] task<T> get_return_object();

      public:
        [[nodiscard]] std::suspend_never initial_suspend();
        [[nodiscard]] final_awaiter final_suspend() noexcept;

      public:
        void unhandled_exception();

      public:
        static void abandon(handle<promise_base>);
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
        handle<promise_base> m_handle;

      public:
        [[nodiscard]] bool await_ready() noexcept;
        [[nodiscard]] std::coroutine_handle<> await_suspend(std::coroutine_handle<>) noexcept;

      public:
        void await_resume() noexcept;
    };

    template <typename T>
    struct task<T>::wake_on_await
    {
        [[nodiscard]] static bool await_ready() noexcept;
        [[nodiscard]] bool await_suspend(std::coroutine_handle<promise_type>) noexcept;

      public:
        static void await_resume() noexcept;
    };

    template <typename T>
    struct task<T>::awaiter
    {
        handle<promise_base> m_handle;

      public:
        [[nodiscard]] bool await_ready() noexcept;
        [[nodiscard]] std::coroutine_handle<> await_suspend(std::coroutine_handle<>) noexcept;

      public:
        [[nodiscard]] T await_resume() noexcept;

      public:
        ~awaiter();
    };
} // namespace coco

#include "task.inl"
