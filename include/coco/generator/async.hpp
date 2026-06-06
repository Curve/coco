#pragma once

#include "../handle/handle.hpp"

#include <variant>
#include <cstdint>

#include <exception>
#include <coroutine>

namespace coco
{
    namespace detail
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
        template <detail::one_of<iterator, void>>
        class awaitable;

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
        enum index : std::uint8_t;

      public:
        std::coroutine_handle<> waiting;
        std::variant<std::monostate, T, std::exception_ptr> value;

      public:
        async_generator get_return_object();

      public:
        static std::suspend_always initial_suspend();
        yield_type final_suspend() noexcept;

      public:
        template <typename U>
            requires std::constructible_from<T, U>
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
    template <detail::one_of<typename async_generator<T>::iterator, void> U>
    class async_generator<T>::awaitable
    {
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
