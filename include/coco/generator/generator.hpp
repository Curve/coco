#pragma once

#include <coroutine>

#include <variant>
#include <exception>

#include <ranges>
#include <iterator>

namespace coco
{
    template <typename T>
    struct generator : std::ranges::view_interface<generator<T>>
    {
        struct iterator;
        struct sentinel;

      public:
        struct promise_type;

      private:
        using handle = std::coroutine_handle<promise_type>;

      private:
        handle m_handle;

      public:
        generator(handle);

      public:
        generator(generator &&) noexcept;
        generator &operator=(generator &&) noexcept;

      public:
        ~generator();

      public:
        [[nodiscard]] iterator begin();
        [[nodiscard]] sentinel end() const;
    };

    template <typename T>
    struct generator<T>::iterator
    {
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = T;

      private:
        handle m_handle;

      public:
        iterator();
        iterator(handle);

      public:
        [[nodiscard]] T &operator*() const;

      public:
        iterator &operator++();
        void operator++(int) const;

      public:
        [[nodiscard]] bool operator==(const sentinel &) const;
    };

    template <typename T>
    struct generator<T>::sentinel
    {
    };

    template <typename T>
    struct generator<T>::promise_type
    {
        std::variant<std::monostate, T, std::exception_ptr> m_value;

      public:
        [[nodiscard]] generator get_return_object();

      public:
        [[nodiscard]] static std::suspend_always initial_suspend();
        [[nodiscard]] static std::suspend_always final_suspend() noexcept;

      public:
        template <typename U>
            requires std::constructible_from<T, U>
        [[nodiscard]] std::suspend_always yield_value(U &&);

      public:
        void return_void();
        void unhandled_exception();
    };
} // namespace coco

#include "generator.inl"
