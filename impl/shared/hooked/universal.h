#pragma once

namespace fd
{
struct hooked_verify_return_address
{
    bool operator()(/*auto&original,*/ char const *module_name) const
    {
        return true;
    }
};
} // namespace fd