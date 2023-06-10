#include "library_info/export.h"
#include "library_info/game_interface.h"
#include "library_info/header.h"
#include "library_info/library.h"
#include "tool/string_view.h"
#include "valve_interface.h"

#include <cassert>

namespace fd
{
valve_interface::valve_interface(string_view name)
{
    assert(0 && "not implemented");
}

valve_interface::valve_interface(string_view name, system_string_view library)
{
    auto lib = (find_library(library));
    auto dos = get_dos(lib);
    auto nt  = get_nt(dos);
    auto fn  = find_export(dos, nt, "CreateInterface");
    ptr_     = find_game_interface(find_root_game_interface(fn), name, false)->get();
}
}