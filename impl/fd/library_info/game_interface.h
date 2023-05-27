#pragma once

#include <cstddef>

namespace fd
{
struct game_interface
{
    void *(*create_fn)();
    char const *name;
    game_interface *next;
    
    void *get() const;
};

game_interface *find_root_game_interface(void *create_func);
game_interface* find_game_interface(game_interface *root_interface,char const *name, size_t length,bool exact);
} // namespace fd