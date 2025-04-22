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

    auto t1 = compute(std::chrono::milliseconds{100});
    auto t2 = compute(std::chrono::milliseconds{200});

    auto then     = clock::now();
    auto [r1, r2] = coco::await(coco::when_all(std::move(t1), std::move(t2)));
    auto time     = clock::now() - then;

    expect(eq(r1, 100));
    expect(eq(r2, 200));

    expect(le(time, std::chrono::milliseconds(250)));

    auto awaitables = std::views::iota(0, 10) //
                      | std::views::transform([](auto i) { return compute(std::chrono::milliseconds{i * 10}); });
    auto awaitables_vec = std::vector<coco::task<long>>{awaitables.begin(), awaitables.end()};

    then         = clock::now();
    auto results = coco::await(coco::when_all(std::move(awaitables_vec)));
    time         = clock::now() - then;

    for (auto i = 0; 10 > i; ++i)
    {
        expect(eq(results[i], i * 10));
    }

    expect(le(time, std::chrono::milliseconds(150)));
};
