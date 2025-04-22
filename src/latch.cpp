#include "latch/latch.hpp"

#include <cassert>

namespace coco
{
    latch::latch(std::ptrdiff_t initial) : counter(initial) {}

    void latch::count_down()
    {
        auto guard = std::lock_guard{mutex};

        assert(counter > 0);

        if (--counter != 0)
        {
            return;
        }

        auto waiting = std::move(handles);

        for (const auto &handle : waiting)
        {
            handle.resume();
        }
    }

    latch::awaiter latch::operator co_await()
    {
        return {this};
    }

    latch::awaiter::awaiter(latch *parent) : m_parent(parent) {}

    bool latch::awaiter::await_ready()
    {
        return false;
    }

    bool latch::awaiter::await_suspend(std::coroutine_handle<> handle)
    {
        auto guard = std::lock_guard{m_parent->mutex};

        if (m_parent->counter == 0)
        {
            return false;
        }

        m_parent->handles.emplace_back(handle);

        return true;
    }

    void latch::awaiter::await_resume() {}
} // namespace coco
