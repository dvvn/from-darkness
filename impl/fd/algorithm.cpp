#include <fd/algorithm.h>

#include <algorithm>
#include <chrono>

namespace fd
{
size_t test_algorithms()
{
    using clock = std::chrono::high_resolution_clock;

    auto run_test = [](auto fn) {
        auto start = clock::now();
        auto val   = fn();
        auto stop  = clock::now();
        return std::pair(val, stop - start);
    };

    const auto set = [](auto& arr, size_t pos, auto... args) {
        ((arr[pos++] = args), ...);
    };

    constexpr auto arrS = 1024 * 1024;

    auto intA       = new int[arrS];
    int  intTestA[] = { 100, 1, 2, 3 };
    set(intA, arrS - 10, 100, 1, 2, 3);

    auto charA       = new char[arrS];
    char charTestA[] = { 100, 1, 2, 3 };
    set(charA, arrS - 10, 100, 1, 2, 3);

    std::vector<void*> dummy;
    auto               a = clock::now();
    dummy.push_back(&a);

#define RUN_TEST(_NAME_, ...)                              \
    [[maybe_unused]] volatile auto _NAME_ = run_test([&] { \
        return __VA_ARGS__;                                \
    });                                                    \
    dummy.insert(dummy.end(), (void*)&_NAME_);

    RUN_TEST(stdFindInt, std::find(intA, intA + arrS, 2));
    RUN_TEST(stdFindChar, std::find(charA, charA + arrS, (char)2));

    RUN_TEST(stdSearchInt, std::search(intA, intA + arrS, std::begin(intTestA), std::end(intTestA)));
    RUN_TEST(stdSearchChar, std::search(charA, charA + arrS, std::begin(charTestA), std::end(charTestA)));

    RUN_TEST(fdFindInt, fd::find(intA, intA + arrS, 2));
    RUN_TEST(fdFindChar, fd::find(charA, charA + arrS, (char)2));
    RUN_TEST(fdFindChar2, fd::find(charA, charA + arrS, 2));

    RUN_TEST(fdSearchInt, fd::find(intA, intA + arrS, std::begin(intTestA), std::end(intTestA)));
    RUN_TEST(fdSearchChar, fd::find(charA, charA + arrS, std::begin(charTestA), std::end(charTestA)));

    return dummy.size();
}

}