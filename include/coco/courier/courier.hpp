#pragma once

#include <memory>
#include <utility>
#include <coroutine>

namespace coco
{
    struct transition;

    struct courier
    {
        friend struct transition;

      protected:
        struct state;

      private:
        std::unique_ptr<state> m_state;

      private:
        courier(std::unique_ptr<state>);

      public:
        courier(courier &&) noexcept;
        courier &operator=(courier &&) noexcept;

      public:
        ~courier();

      public:
        void schedule() &&;

      public:
        static std::pair<courier, transition> create(bool cancel = false);
    };

    struct transition
    {
        friend struct courier;

      private:
        struct awaiter;

      private:
        using state = courier::state;

      private:
        state *m_state;

      private:
        transition(state *);

      public:
        transition(transition &&) noexcept;
        transition &operator=(transition &&) noexcept;

      public:
        [[nodiscard]] awaiter operator co_await() &&;

      public:
        static transition noop();
    };

    struct transition::awaiter
    {
        using state = transition::state;

      public:
        state *m_state;

      public:
        bool await_ready() const noexcept; // NOLINT(*-nodiscard)
        void await_suspend(std::coroutine_handle<>) const noexcept;

      public:
        void await_resume();
    };
} // namespace coco
