#include <thread>

#include <boost/ut.hpp>

#include <coco/task/task.hpp>
#include <coco/utils/utils.hpp>
#include <coco/courier/courier.hpp>

using namespace boost::ut;

// NOLINTNEXTLINE
suite<"transitioner"> transitioner_test = []
{
    static const auto spawn_thread = [](coco::courier courier)
    {
        std::thread{[courier = std::move(courier)]() mutable { std::move(courier).schedule(); }}.detach();
    };

    auto [courier, transition] = coco::courier::create();

    spawn_thread(std::move(courier));
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    static const auto task = [](auto id, auto transition) -> coco::task<void>
    {
        co_await std::move(transition);
        expect(neq(id, std::this_thread::get_id()));
    };

    coco::await(task(std::this_thread::get_id(), std::move(transition)));

    static const auto noop_task = [](auto id, auto transition) -> coco::task<void>
    {
        co_await std::move(transition);
        expect(eq(id, std::this_thread::get_id()));
    };

    coco::await(noop_task(std::this_thread::get_id(), coco::transition::noop()));
};
