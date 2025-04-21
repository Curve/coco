#include "sleep.hpp"

#include <future>

#include <boost/ut.hpp>
#include <coco/stray/stray.hpp>

using namespace boost::ut;
using namespace coco::tests;

// NOLINTNEXTLINE
suite<"basic"> basic_test = []
{
    static auto compute = [](std::promise<void> result) -> coco::stray
    {
        co_await co_sleep{std::chrono::milliseconds(500)};
        result.set_value();
    };

    auto promise = std::promise<void>{};
    auto fut     = promise.get_future();

    compute(std::move(promise));

    expect(fut.wait_for(std::chrono::seconds(1)) == std::future_status::ready);
};
