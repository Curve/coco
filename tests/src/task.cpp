#include "sleep.hpp"

#include <boost/ut.hpp>
#include <coco/task/task.hpp>

using namespace boost::ut;
using namespace coco::tests;

// NOLINTNEXTLINE
suite<"task"> task_test = []
{
    "eager"_test = []
    {
        static auto compute = []() -> coco::task<int>
        {
            co_await co_sleep{std::chrono::milliseconds{500}};
            co_return 10;
        };

        static auto await = [](std::promise<int> promise) -> coco::basic_task
        {
            auto result = co_await compute();
            promise.set_value(result + 10);
        };

        "eager"_test = []
        {
            auto task = compute();

            const auto &future = static_cast<const std::future<int> &>(task);
            auto status        = future.wait_for(std::chrono::seconds(1));

            expect(status == std::future_status::ready);
            expect(eq(task.get(), 10));
        };

        "get"_test = []
        {
            expect(eq(compute().get(), 10));
        };

        "co_await"_test = []
        {
            auto promise = std::promise<int>{};
            auto future  = promise.get_future();

            await(std::move(promise)).get();

            expect(future.wait_for(std::chrono::seconds(0)) == std::future_status::ready);
            expect(future.get() == 20);
        };

        "then"_test = []
        {
            auto promise = std::promise<int>{};
            auto future  = promise.get_future();

            compute().then(
                [promise = std::move(promise)](auto result) mutable
                {
                    expect(eq(result, 10));
                    promise.set_value(result);
                });

            expect(eq(future.get(), 10));
        };
    };

    "lazy"_test = []
    {
        static auto compute = []() -> coco::task<int>
        {
            co_await coco::task<int>::idle{};
            co_return 10;
        };

        static auto await = [](coco::task<int> &awaitable) -> coco::task<int>
        {
            co_return co_await std::move(awaitable);
        };

        "not-eager"_test = []
        {
            auto task = compute();

            const auto &future = static_cast<const std::future<int> &>(task);
            auto status        = future.wait_for(std::chrono::seconds(1));

            expect(status != std::future_status::ready);
            expect(eq(await(task).get(), 10));
        };
    };
};
