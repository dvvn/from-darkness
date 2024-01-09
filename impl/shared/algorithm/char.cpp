#include "algorithm/char.h"

namespace fd
{
namespace detail
{

islower_table::islower_table()
    : basic_char_table{default_char_table_v<bool>}
{
    set('a', 'z', true);
}

isupper_table::isupper_table()
    : basic_char_table{default_char_table_v<bool>}
{
    set('A', 'Z', true);
}

isdigit_table::isdigit_table()
    : basic_char_table{default_char_table_v<bool>}
{
    set('0', '9', true);
}

isxdigit_table::isxdigit_table()
    : basic_char_table{default_char_table_v<bool>}
{
    set('0', '9', true);
    set('a', 'f', true);
    set('A', 'F', true);
}

tolower_table::tolower_table()
    : basic_char_table{default_char_table_v<char>}
{
    set('A', 'Z', 'a', 'z');
}

toupper_table::toupper_table()
    : basic_char_table{default_char_table_v<char>}
{
    set('a', 'z', 'A', 'Z');
}
} // namespace detail

char_table_wrapper<detail::islower_table> const islower;
char_table_wrapper<detail::isupper_table> const isupper;
char_table_wrapper<detail::isdigit_table> const isdigit;
char_table_wrapper<detail::isxdigit_table> const isxdigit;

char_table_wrapper<detail::tolower_table> const tolower;
char_table_wrapper<detail::toupper_table> const toupper;
} // namespace fd