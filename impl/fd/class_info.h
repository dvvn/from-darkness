#pragma once

namespace fd
{
template <template <typename...> class T>
constexpr bool has_correct_destuction_order()
{
    char val;

    class helper
    {
        char* v;
        char  setTo;

      public:
        constexpr helper(char& v, char setTo)
            : v(&v)
            , setTo(setTo)
        {
        }

        constexpr ~helper()
        {
            *v = setTo;
        }
    };

    {
        auto const _ = T(helper(val, 1), helper(val, 2));
        val          = 0;
    }
    return val == 1;
}
}