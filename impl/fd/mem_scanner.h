#pragma once

#include <functional>

namespace fd
{
struct basic_find_callback
{
    virtual bool call(void *result) = 0;

    basic_find_callback *base()
    {
        return this;
    }
};

template <typename T>
class find_callback_caller : public basic_find_callback
{
    bool call(void *result) override
    {
        return call(reinterpret_cast<T>(result));
    }

  public:
    virtual bool call(T result) = 0;
};

template <>
class find_callback_caller<void *> : public basic_find_callback
{
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

template <typename T, std::invocable<T> Fn>
class find_callback : public find_callback_caller<T>
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

    bool call(T result) override
    {
        return fn_(result);
    }
};

template <typename T, typename Fn>
class find_callback<T, Fn &> : public find_callback_caller<T>
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

    bool call(T result) override
    {
        return (*fn_)(result);
    }
};

template <typename T, typename Fn>
class find_callback<T, Fn &&>;

template <typename T, typename Fn>
using find_callback_ref = find_callback<
    T, //
    std::conditional_t<std::is_pointer_v<Fn>, Fn, std::add_lvalue_reference_t<Fn>>>;

template <typename Fn>
find_callback(Fn) -> find_callback<typename function_argument<Fn>::type, std::decay_t<Fn>>;

// template <typename T, typename Fn>
// find_callback(std::in_place_type_t<T>, Fn) -> find_callback<T, std::decay_t<Fn>>;

void *find_pattern(void *begin, void *end, char const *pattern, size_t pattern_length);
bool find_pattern(void *begin, void *end, char const *pattern, size_t pattern_length, basic_find_callback *callback);

template <typename Fn>
bool find_pattern(void *begin, void *end, char const *pattern, size_t pattern_length, Fn callback)
{
   return find_pattern(begin, end, pattern, pattern_length, find_callback_ref<void *, Fn>(callback).base());
}

uintptr_t find_xref(void *begin, void *end, uintptr_t &address);
bool find_xref(void *begin, void *end, uintptr_t &address, basic_find_callback *callback);

template <typename Fn>
bool find_xref(void *begin, void *end, uintptr_t &address, Fn callback)
{
    return find_xref(begin, end, address, find_callback_ref<uintptr_t &, Fn>(callback).base());
}

bool find_bytes(void *begin, void *end, void *bytes, size_t length, basic_find_callback *callback);

template <typename Fn>
bool find_bytes(void *begin, void *end, void *bytes, size_t length, Fn callback)
{
    return find_bytes(begin, end, bytes, length, find_callback_ref<void *, Fn>(callback).base());
}
} // namespace fd