#include <ranges>
#include <numeric>

#include <boost/ut.hpp>
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

    "iterate"_test = []
    {
        auto i = 0;

        for (const auto &value : generate())
        {
            expect(eq(value, i++));
        }

        expect(eq(i, 10));
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
