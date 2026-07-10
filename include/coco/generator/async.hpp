#pragma once

#include "../handle/handle.hpp"

#include <atomic>
#include <variant>

#include <exception>
#include <coroutine>

namespace coco
{
    namespace detail::of_generator
    {
        template <typename T, typename... Ts>
        concept one_of = (std::same_as<T, Ts> || ...);
    }

    template <typename T>
    struct async_generator
    {
        struct promise_type;

      public:
        struct iterator;
        struct sentinel;

      private:
        template <typename>
        struct awaitable;

      private:
        handle<promise_type> m_handle;

      public:
        async_generator(handle<promise_type>);

      public:
        async_generator(async_generator &&) noexcept;
        async_generator &operator=(async_generator &&) noexcept;

      public:
        ~async_generator();

      public:
        [[nodiscard]] awaitable<iterator> begin();
        [[nodiscard]] sentinel end() const;
    };

    template <typename T>
    struct async_generator<T>::promise_type
    {
        struct yield_type;

      public:
        std::atomic<std::coroutine_handle<>> waiting;
        std::variant<std::monostate, T, std::exception_ptr> value;

      public:
        async_generator get_return_object();

      public:
        static std::suspend_always initial_suspend();
        yield_type final_suspend() noexcept;

      public:
        template <std::convertible_to<T> U>
        yield_type yield_value(U &&);

      public:
        void return_void() noexcept;
        void unhandled_exception() noexcept;
    };

    template <typename T>
    struct async_generator<T>::iterator
    {
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = T;

      private:
        handle<promise_type> m_handle;

      public:
        iterator();
        iterator(handle<promise_type>);

      public:
        [[nodiscard]] T &operator*() const;

      public:
        [[nodiscard]] awaitable<void> operator++();
        [[nodiscard]] bool operator==(const sentinel &) const;
    };

    template <typename T>
    struct async_generator<T>::sentinel
    {
    };

    template <typename T>
    struct async_generator<T>::promise_type::yield_type
    {
        handle<promise_type> m_handle;

      public:
        static bool await_ready() noexcept;
        std::coroutine_handle<> await_suspend(std::coroutine_handle<>) noexcept;

      public:
        void await_resume() noexcept;
    };

    template <typename T>
    template <typename U>
    struct async_generator<T>::awaitable
    {
        static_assert(detail::of_generator::one_of<U, iterator, void>);

      private:
        handle<promise_type> m_handle;

      public:
        awaitable(handle<promise_type>);

      public:
        static bool await_ready() noexcept;
        std::coroutine_handle<> await_suspend(std::coroutine_handle<>) noexcept;

      public:
        U await_resume() noexcept;
    };
} // namespace coco

#include "async.inl"
