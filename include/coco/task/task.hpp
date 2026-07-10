#pragma once

#include "../handle/handle.hpp"

#include <exception>

#include <atomic>
#include <variant>

namespace coco
{
    struct task_options
    {
        bool lazy{false};
    };

    namespace detail::of_task
    {
        template <typename T, task_options>
        struct promise_type;
    }

    template <typename T, task_options opts = {}>
    class task
    {
        friend detail::of_task::promise_type<T, opts>;

      private:
        struct awaiter;
        struct promise_base;

      public:
        struct promise_type;

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
        [[nodiscard]] awaiter operator co_await() &&;
    };

    template <typename T, task_options opts>
    struct task<T, opts>::promise_base
    {
        struct final_awaiter;

      public:
        using result = std::conditional_t<std::is_void_v<T>, std::monostate, T>;

      public:
        std::atomic<void *> continuation{};
        std::variant<std::monostate, result, std::exception_ptr> value;

      public:
        task<T, opts> get_return_object();

      public:
        auto initial_suspend();
        final_awaiter final_suspend() noexcept;

      public:
        void unhandled_exception();

      public:
        static void abandon(handle<promise_base>);
    };

    template <typename T, task_options opts>
    struct detail::of_task::promise_type : task<T, opts>::promise_base
    {
        void return_value(T);
    };

    template <task_options opts>
    struct detail::of_task::promise_type<void, opts> : coco::task<void, opts>::promise_base
    {
        void return_void();
    };

    template <typename T, task_options opts>
    struct task<T, opts>::promise_type : detail::of_task::promise_type<T, opts>
    {
    };

    template <typename T, task_options opts>
    struct task<T, opts>::promise_base::final_awaiter
    {
        handle<promise_base> m_handle;

      public:
        bool await_ready() noexcept;
        std::coroutine_handle<> await_suspend(std::coroutine_handle<>) noexcept;

      public:
        void await_resume() noexcept;
    };

    template <typename T, task_options opts>
    struct task<T, opts>::awaiter
    {
        handle<promise_base> m_handle;

      public:
        bool await_ready() noexcept;
        std::coroutine_handle<> await_suspend(std::coroutine_handle<>) noexcept;

      public:
        T await_resume();

      public:
        ~awaiter();
    };
} // namespace coco

#include "task.inl"
