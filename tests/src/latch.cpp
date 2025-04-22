#include <array>
#include <thread>

#include <boost/ut.hpp>
#include <coco/task/task.hpp>
#include <coco/latch/latch.hpp>
#include <coco/utils/utils.hpp>

using namespace boost::ut;

// NOLINTNEXTLINE
suite<"latch"> latch_test = []
{
    static constexpr auto N = 5;
    static auto finished    = std::array<bool, N>{};

    static const auto spawn_thread = [](coco::latch &latch, std::size_t index)
    {
        std::thread t{[&latch, index]()
                      {
                          std::this_thread::sleep_for(std::chrono::milliseconds(index * 100));
                          finished[index] = true;
                          latch.count_down();
                      }};

        t.detach();
    };

    coco::latch latch{N};

    for (auto i = 0u; N > i; ++i)
    {
        spawn_thread(latch, i);
    }

    coco::await(latch);

    for (auto i = 0u; N > i; ++i)
    {
        expect(finished[i]);
    }

    coco::await(latch);

    expect(true);
};
