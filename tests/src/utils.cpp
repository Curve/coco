#include "sleep.hpp"

#include <ranges>

#include <boost/ut.hpp>
#include <coco/task/task.hpp>
#include <coco/utils/utils.hpp>

using namespace boost::ut;
using namespace coco::tests;

// NOLINTNEXTLINE
suite<"utils"> utils_test = []
{
    using clock = std::chrono::high_resolution_clock;

    static auto compute = [](std::chrono::milliseconds ms) -> coco::task<long>
    {
        co_await co_sleep{ms};
        co_return ms.count();
    };

    auto t1 = compute(std::chrono::milliseconds{150});
    auto t2 = compute(std::chrono::milliseconds{100});
    auto t3 = compute(std::chrono::milliseconds{50});

    auto then         = clock::now();
    auto [r1, r2, r3] = coco::await(coco::when_all(std::move(t1), std::move(t2), std::move(t3)));
    auto time         = clock::now() - then;

    expect(eq(r1, 150));
    expect(eq(r2, 100));
    expect(eq(r3, 50));

    expect(le(time, std::chrono::milliseconds(200)));

    std::vector<coco::task<long>> tasks;

    for (auto i = 0; 10 > i; ++i)
    {
        tasks.emplace_back(compute(std::chrono::milliseconds{i * 10}));
    }

    then         = clock::now();
    auto results = coco::await(coco::when_all(std::move(tasks)));
    time         = clock::now() - then;

    for (auto i = 0; 10 > i; ++i)
    {
        expect(eq(results[i], i * 10));
    }

    expect(le(time, std::chrono::milliseconds(150)));
};
