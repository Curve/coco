#include <thread>

#include <boost/ut.hpp>
#include <coco/task/task.hpp>
#include <coco/sync/sync.hpp>
#include <coco/promise/promise.hpp>

using namespace boost::ut;

// NOLINTNEXTLINE
suite<"promise"> promise_test = []
{
    static auto compute = []()
    {
        auto promise = coco::promise<int>{};
        auto future  = promise.get_future();

        std::thread thread{[promise = std::move(promise)]() mutable
                           {
                               std::this_thread::sleep_for(std::chrono::milliseconds(500));
                               promise.set_value(10);
                           }};

        thread.detach();

        return future;
    };

    static auto await = []() -> coco::task<int>
    {
        auto result = co_await compute();
        co_return result + 10;
    };

    "get"_test = []
    {
        expect(eq(coco::await(compute()), 10));
    };

    "co_await"_test = []
    {
        expect(eq(coco::await(await()), 20));
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
};
