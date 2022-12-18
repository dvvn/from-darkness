#pragma once

namespace fd
{
    template <typename T>
    struct basic_variable
    {
        virtual ~basic_variable() = default;

        virtual T& get_ref() = 0;
        virtual T get() const = 0;
        virtual void set(T&& value) = 0;
        virtual void set(const T& value) = 0;
    };
}