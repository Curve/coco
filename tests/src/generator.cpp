#include "sleep.hpp"

#include <ranges>
#include <numeric>

#include <boost/ut.hpp>

#include <coco/task/task.hpp>
#include <coco/utils/utils.hpp>

#include <coco/generator/async.hpp>
#include <coco/generator/generator.hpp>

using namespace boost::ut;

// NOLINTNEXTLINE
suite<"generator"> generator_test = []
{
    static auto generate = []() -> coco::generator<int>
    {
        for (auto i = 0; 10 > i; ++i)
        {
            co_yield i;
        }
    };

    static auto async_generate = []() -> coco::async_generator<int>
    {
        for (auto i = 0; 10 > i; ++i)
        {
            co_await coco::tests::co_sleep{std::chrono::milliseconds{150}};
            co_yield i;
        }
    };

    static auto async_iterate = []() -> coco::task<std::pair<int, unsigned>>
    {
        auto i   = 0u;
        auto sum = 0;

        auto gen = async_generate();
        auto end = gen.end();

        for (auto it = co_await gen.begin(); it != end; co_await ++it)
        {
            ++i;
            sum += *it;
        }

        co_return std::make_pair(sum, i);
    };

    "iterate"_test = []
    {
        auto i = 0;

        for (const auto &value : generate())
        {
            expect(eq(value, i++));
        }

        expect(eq(i, 10));
    };

    "async"_test = []
    {
        auto [sum, iterations] = coco::await(async_iterate());
        expect(eq(sum, 45));
        expect(eq(iterations, 10u));
    };

    "ranges"_test = []
    {
        auto filtered = generate()                                              //
                        | std::views::filter([](auto x) { return x % 2 == 0; }) //
                        | std::views::common;

        auto sum = std::accumulate(filtered.begin(), filtered.end(), 0);

        expect(eq(sum, 20));
    };

    "find"_test = []
    {
        auto num = generate().find(5);

        expect(num.has_value());
        expect(eq(num.value(), 5));
    };

    "skip"_test = []
    {
        auto num = generate().skip(0);

        expect(num.has_value());
        expect(eq(num.value(), 1));
    };
};
