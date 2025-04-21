#pragma once

#include <coroutine>

namespace coco
{
    template <typename T>
    class handle
    {
        using handle_t = std::coroutine_handle<T>;

      private:
        handle_t m_handle;

      public:
        handle();
        handle(handle_t);

      public:
        handle(const handle &);
        handle(handle &&) noexcept;

      public:
        handle &operator=(const handle &);
        handle &operator=(handle &&) noexcept;

      public:
        [[nodiscard]] bool done() const;
        [[nodiscard]] handle_t get() const;

      public:
        void resume() const;
        void destroy() const;

      public:
        [[nodiscard]] auto *operator->() const;
        [[nodiscard]] explicit operator bool() const;

      public:
        static handle from(T *);
    };
} // namespace coco

#include "handle.inl"
