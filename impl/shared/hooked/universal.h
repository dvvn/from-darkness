#pragma once

namespace fd
{
struct hooked_verify_return_address
{
    using function_type = char(__thiscall *)(void *, char const *);

    bool operator()(/*auto &original,*/ void const *retaddr) const
    {
        return true;
    }
};
} // namespace fd