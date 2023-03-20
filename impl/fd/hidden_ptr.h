#pragma once

#include <compare>
#include <utility>

namespace fd
{

class hidden_ptr
{
    union
    {
        uintptr_t addr_;
        void *ptr_;
        void const *cptr_;
    };

  public:
    hidden_ptr(void const *ptr)
        : cptr_(ptr)
    {
    }

    hidden_ptr operator+(std::integral auto val) const
    {
        auto ret = *this;
        ret.addr_ += val;
        return ret;
    }

    hidden_ptr operator-(std::integral auto val) const
    {
        auto ret = *this;
        ret.addr_ -= val;
        return ret;
    }

    hidden_ptr operator*(std::integral auto val) const
    {
        auto ret = *this;
        ret.addr_ *= val;
        return ret;
    }

    hidden_ptr operator/(std::integral auto val) const
    {
        auto ret = *this;
        ret.addr_ /= val;
        return ret;
    }

    std::strong_ordering operator<=>(std::integral auto other) const
    {
        return addr_ <=> other;
    }

    template <typename T>
    T as() const
    {
        if constexpr (std::is_pointer_v<T>)
            return static_cast<T>(ptr_);
        else if constexpr (std::is_lvalue_reference_v<T>)
            return *static_cast<std::remove_reference_t<T> *>(ptr_);
        else
        {
            //static_assert(sizeof(T) == sizeof(uintptr_t));

            union
            {
                uintptr_t addr;
                T fn;
            };

            addr = addr_;
            return fn;
        }
    }

    template <typename T, typename... Args>
    decltype(auto) as_fn(Args &&...args) const
    {
        return as<T>()(std::forward<Args>(args)...);
    }

    operator hidden_ptr() const = delete;

    template <typename T>
    operator T *() const
    {
        return as<T *>();
    }

    template <typename T>
    operator T &() const requires(!std::is_member_function_pointer_v<T>)
    {
        return as<T &>();
    }

    template <typename T>
    operator T() const requires(std::is_member_function_pointer_v<T>)
    {
        return as<T>();
    }

    hidden_ptr operator*() const
    {
        return *static_cast<void **>(ptr_);
    }
};

} // namespace fd