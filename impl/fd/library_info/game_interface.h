#pragma once

#include <cstddef>

using LDR_DATA_TABLE_ENTRY = struct _LDR_DATA_TABLE_ENTRY;

namespace fd
{
struct game_interface
{
    void *(*create_fn)();
    char const *name;
    game_interface *next;

    game_interface operator+(size_t offset) const;
    void *get() const;
};

struct found_game_interface
{
    game_interface *i;
    bool identical_name;

    explicit operator bool() const
    {
        return i != nullptr;
    }
};

game_interface *find_root_game_interface(void *create_func);
found_game_interface find_game_interface(char const *name, size_t length, game_interface *root_interface);
} // namespace fd