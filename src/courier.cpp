#include "courier/courier.hpp"

#include <atomic>
#include <cassert>

namespace coco
{
    struct courier::state
    {
        bool cancel;
        std::atomic<std::coroutine_handle<>> handle;
    };

    courier::courier(std::unique_ptr<state> state) : m_state(std::move(state)) {}

    courier::courier(courier &&) noexcept = default;

    courier &courier::operator=(courier &&) noexcept = default;

    courier::~courier()
    {
        if (!m_state)
        {
            return;
        }

        auto handle = m_state->handle.load(std::memory_order_acquire);

        if (!handle)
        {
            assert(false && "transition was never awaited");
            return;
        }

        if (m_state->cancel)
        {
            handle.destroy();
            return;
        }

        assert(handle == std::noop_coroutine() && "schedule() was never called");
    }

    void courier::schedule() &&
    {
        m_state->handle.wait({});
        m_state->handle.exchange(std::noop_coroutine(), std::memory_order_acq_rel).resume();
    }

    std::pair<courier, transition> courier::create(bool cancel)
    {
        auto data       = std::make_unique<state>(cancel);
        auto *const ptr = data.get();

        return {courier{std::move(data)}, transition{ptr}};
    }

    transition::transition(state *state) : m_state(state) {}

    transition::transition(transition &&) noexcept = default;

    transition &transition::operator=(transition &&) noexcept = default;

    transition::awaiter transition::operator co_await() &&
    {
        return {m_state};
    }

    bool transition::awaiter::await_ready() const noexcept // NOLINT(*-static)
    {
        return !m_state;
    }

    void transition::awaiter::await_suspend(std::coroutine_handle<> handle) const noexcept
    {
        if (!m_state)
        {
            return;
        }

        m_state->handle.store(handle, std::memory_order_release);
        m_state->handle.notify_one();
    }

    void transition::awaiter::await_resume() {}

    transition transition::noop()
    {
        return {nullptr};
    }
} // namespace coco
