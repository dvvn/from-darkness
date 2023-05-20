#pragma once

#include <functional>

namespace fd
{
template <typename T>
size_t netvar_hash(const T &value, std::hash<T> fn = {})
{
    return fn(value);
}


}