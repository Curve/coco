#include "latch/latch.hpp"

namespace coco
{
    latch::latch(std::ptrdiff_t initial) : counter(initial) {}

    void latch::count_down()
    {
        auto guard = std::lock_guard{mutex};

        if (--counter > 0)
        {
            return;
        }

        for (const auto &handle : handles)
        {
            handle.resume();
        }

        handles.clear();
    }

    latch::awaiter latch::operator co_await()
    {
        return {this};
    }

    latch::awaiter::awaiter(latch *parent) : m_parent(parent) {}

    bool latch::awaiter::await_ready() const
    {
        auto guard = std::lock_guard{m_parent->mutex};
        return m_parent->counter == 0;
    }

    bool latch::awaiter::await_suspend(std::coroutine_handle<> handle)
    {
        auto guard = std::lock_guard{m_parent->mutex};

        if (!m_parent->counter)
        {
            return false;
        }

        m_parent->handles.emplace_back(handle);

        return true;
    }

    void latch::awaiter::await_resume() {}
} // namespace coco
