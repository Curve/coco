#include "sleep.hpp"

#include <boost/ut.hpp>
#include <coco/task/task.hpp>
#include <coco/utils/utils.hpp>

using namespace boost::ut;
using namespace coco::tests;

// NOLINTNEXTLINE
suite<"task"> task_test = []
{
    static auto compute = []() -> coco::task<int>
    {
        co_await co_sleep{std::chrono::milliseconds{500}};
        co_return 10;
    };

    static auto await = [](std::promise<int> promise) -> coco::stray
    {
        auto result = co_await compute();
        promise.set_value(result + 10);
    };

    "discard"_test = []
    {
        compute();
    };

    "get"_test = []
    {
        expect(eq(coco::await(compute()), 10));
    };

    "co_await"_test = []
    {
        auto promise = std::promise<int>{};
        auto future  = promise.get_future();

        await(std::move(promise));

        expect(future.get() == 20);
    };

    "then"_test = []
    {
        auto promise = std::promise<int>{};
        auto future  = promise.get_future();

        coco::then(compute(),
                   [promise = std::move(promise)](auto result) mutable
                   {
                       expect(eq(result, 10));
                       promise.set_value(result);
                   });

        expect(eq(future.get(), 10));
    };

    static auto eager = [](bool &flag) -> coco::task<void> // NOLINT(*-coroutine-parameters)
    {
        flag = true;
        co_return;
    };

    "eager"_test = []
    {
        auto result = false;
        auto task   = eager(result);

        expect(eq(result, true));
    };

    static auto lazy = [](bool &flag) -> coco::task<void> // NOLINT(*-coroutine-parameters)
    {
        co_await coco::task<void>::wake_on_await{};
        flag = true;
    };

    "lazy"_test = []
    {
        auto result = false;
        auto task   = lazy(result);

        expect(eq(result, false));
        coco::await(std::move(task));
        expect(eq(result, true));
    };
};
