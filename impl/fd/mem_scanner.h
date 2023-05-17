#pragma once

#include <functional>

namespace fd
{
template <typename T>
struct basic_find_callback
{
    using argument_type = T;

    virtual ~basic_find_callback()          = default;
    virtual bool operator()(T result) const = 0;
};

template <typename Fn>
struct function_argument : function_argument<decltype(std::function(std::declval<Fn>()))>
{
};

template <typename Ret, typename A>
struct function_argument<std::function<Ret(A)>>
{
    using type = A;
};

template <typename T, typename Fn>
class find_callback : public basic_find_callback<T>
{
    Fn fn_;

  public:
    find_callback(std::in_place_type_t<T>, Fn fn)
        : fn_(std::move(fn))
    {
    }

    find_callback(Fn fn)
        : fn_(std::move(fn))
    {
    }

    bool operator()(T result) const override
    {
        return fn_(result);
    }

    /*template <typename Q>
    operator basic_find_callback<Q>() const requires(!std::same_as<T, Q> && std::convertible_to<T, Q>)
    {
    }*/
};

template <typename Fn>
find_callback(Fn) -> find_callback<typename function_argument<Fn>::type, std::decay_t<Fn>>;

//template <typename T, typename Fn>
//find_callback(std::in_place_type_t<T>, Fn) -> find_callback<T, std::decay_t<Fn>>;

void *find_pattern(void *begin, void *end, char const *pattern, size_t pattern_length);
void find_pattern(
    void *begin,
    void *end,
    char const *pattern,
    size_t pattern_length,
    basic_find_callback<void *> const &callback);

uintptr_t find_xref(void *begin, void *end, uintptr_t &address);
void find_xref(void *begin, void *end, uintptr_t &address, basic_find_callback<uintptr_t &> const &callback);

bool find_bytes(void *begin, void *end, void *bytes, size_t length, basic_find_callback<void *> const &callback);
} // namespace fd