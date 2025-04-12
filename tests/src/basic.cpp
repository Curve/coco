#include "sleep.hpp"

#include <boost/ut.hpp>
#include <coco/basic/basic.hpp>

using namespace boost::ut;
using namespace coco::tests;

// NOLINTNEXTLINE
suite<"basic"> basic_test = []
{
    static auto compute = []() -> coco::basic_task
    {
        co_await co_sleep{std::chrono::milliseconds(500)};
    };

    "get"_test = []
    {
        compute().get();
    };
};
