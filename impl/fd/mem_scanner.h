#pragma once

#include <functional>

namespace fd
{
template <typename T>
struct basic_find_callback
{
    using argument_type = T;

    //virtual ~basic_find_callback()          = default;
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

    basic_find_callback<T> const &base() const
    {
        return *this;
    }
};

template <typename T, typename Fn>
class find_callback<T, Fn &> : public basic_find_callback<T>
{
    Fn *fn_;

  public:
    find_callback(Fn *fn)
        : fn_(fn)
    {
    }

    find_callback(Fn &fn)
        : fn_(&fn)
    {
    }

    bool operator()(T result) const override
    {
        return (*fn_)(result);
    }

    basic_find_callback<T> const &base() const
    {
        return *this;
    }
};

template <typename Fn>
find_callback(Fn) -> find_callback<typename function_argument<Fn>::type, std::decay_t<Fn>>;

// template <typename T, typename Fn>
// find_callback(std::in_place_type_t<T>, Fn) -> find_callback<T, std::decay_t<Fn>>;

void *find_pattern(void *begin, void *end, char const *pattern, size_t pattern_length);
void find_pattern(
    void *begin,
    void *end,
    char const *pattern,
    size_t pattern_length,
    basic_find_callback<void *> const &callback);

template <typename Fn>
void find_pattern(void *begin, void *end, char const *pattern, size_t pattern_length, Fn callback)
{
    callback(begin, end, pattern, pattern_length, find_callback<void *, Fn &>(callback).base());
}

uintptr_t find_xref(void *begin, void *end, uintptr_t &address);
bool find_xref(void *begin, void *end, uintptr_t &address, basic_find_callback<uintptr_t &> const &callback);

template <typename Fn>
bool find_xref(void *begin, void *end, uintptr_t &address, Fn callback)
{
    return find_xref(begin, end, address, find_callback<uintptr_t &, Fn &>(callback).base());
}

bool find_bytes(void *begin, void *end, void *bytes, size_t length, basic_find_callback<void *> const &callback);

template <typename Fn>
bool find_bytes(void *begin, void *end, void *bytes, size_t length, Fn callback)
{
    return find_bytes(begin, end, bytes, length, find_callback<void *, Fn &>(callback).base());
}
} // namespace fd