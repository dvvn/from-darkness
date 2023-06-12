#include "export.h"
#include "game_interface.h"
#include "header.h"
#include "library.h"

#include <fd/tool/string_view.h>

#include <algorithm>
#include <cassert>
#include <cctype>

namespace fd
{
void *game_interface::get() const
{
    return create_fn();
}

game_interface *find_root_game_interface(void *create_func)
{
    assert(create_func != nullptr);

    auto relative_fn  = reinterpret_cast<uintptr_t>(create_func) + 0x5;
    auto displacement = *reinterpret_cast<int32_t *>(relative_fn);
    auto jmp          = relative_fn + sizeof(int32_t) + displacement;

    return **reinterpret_cast<game_interface ***>(jmp + 0x6);
}

game_interface *find_root_game_interface(system_string_view source)
{
    auto lib = find_library(source);
    auto dos = get_dos(lib);
    auto nt  = get_nt(dos);
    return find_root_game_interface(find_export(dos, nt, "CreateInterface"));
}

template <bool Exact>
static game_interface *find_game_interface(
    game_interface *begin,
    game_interface *end,
    char const *name,
    size_t length,
    std::bool_constant<Exact> = {})
{
    // unsafe comparison, but ok

    for (; begin != end; begin = begin->next)
    {
        if constexpr (Exact)
        {
            if (begin->name[length] != '\0')
                continue;
        }
        else
        {
            if (begin->name[length] != '\0' && !isdigit(begin->name[length]))
                continue;
        }

        if (memcmp(begin->name, name, length) == 0)
            break;
    }

#ifdef _DEBUG
    if constexpr (!Exact)
    {
        if (begin != end && begin->name[length] != '\0')
        {
            for (auto c = begin->name + length + 1; *c != '\0'; ++c)
            {
                if (!isdigit(*c))
                {
                    assert(0 && "provided incorrect name");
                    return end;
                }
            }

            for (auto it = begin->next; it != end; it = it->next)
            {
                if (it->name[length] == '\0' || isdigit(it->name[length]))
                {
                    assert(0 && "provided duplicate name");
                    return end;
                }
            }
        }
    }
#endif

    return begin;
}

#define FIND_GAME_INTERFACE(_X_) find_game_interface<_X_>(root_interface, nullptr, name, length)

game_interface *find_game_interface(game_interface *root_interface, char const *name, size_t length, bool exact)
{
    return exact ? FIND_GAME_INTERFACE(true) : FIND_GAME_INTERFACE(false);
}

#undef FIND_GAME_INTERFACE

game_interface *find_game_interface(game_interface *root_interface, string_view name, bool exact)
{
    return find_game_interface(root_interface, name.data(), name.length(), exact);
}
} // namespace fd