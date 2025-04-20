#pragma once

#include "../basic/basic.hpp"

#include <memory>

namespace coco
{
    template <typename>
    class future;

    template <typename T>
    struct promise_base
    {
        template <typename>
        friend class future;

      protected:
        struct state;

      protected:
        std::promise<T> m_promise;
        std::shared_ptr<state> m_state;

      public:
        promise_base();

      public:
        promise_base(promise_base &&) noexcept;
        promise_base &operator=(promise_base &&) noexcept;

      public:
        ~promise_base();

      protected:
        void resume();

      public:
        [[nodiscard]] future<T> get_future();

      public:
        void set_exception(std::exception_ptr);
    };

    template <typename T>
    struct promise : promise_base<T>
    {
        void set_value(T);
    };

    template <>
    struct promise<void> : promise_base<void>
    {
        void set_value();
    };

    template <typename T>
    class future
    {
        template <typename>
        friend struct promise_base;

      private:
        struct awaiter;

      private:
        using state = promise_base<T>::state;

      private:
        std::future<T> m_future;
        std::shared_ptr<state> m_state;

      private:
        future(std::future<T>, std::shared_ptr<state>);

      public:
        future();

      public:
        future(future &&) noexcept;
        future &operator=(future &&) noexcept;

      public:
        [[nodiscard]] T get();

      public:
        template <typename Callback>
        basic_task then(Callback) &&;

      public:
        [[nodiscard]] awaiter operator co_await() &&;
        [[nodiscard]] operator const std::future<T> &() &;
    };

    template <typename T>
    struct future<T>::awaiter
    {
        using state = promise_base<T>::state;

      public:
        std::future<T> m_future;
        std::shared_ptr<state> m_state;

      public:
        [[nodiscard]] bool await_ready() const noexcept;
        [[nodiscard]] bool await_suspend(std::coroutine_handle<>) noexcept;

      public:
        [[nodiscard]] T await_resume();
    };
} // namespace coco

#include "promise.inl"
