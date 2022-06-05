#include <fds/tools/interface.h>

#include <d3d9.h>

import fds.csgo.modules;

using namespace fds;

FDS_INTERFACE_IMPL(IDirect3DDevice9, csgo_modules::shaderapidx9.find_interface_sig<"A1 ? ? ? ? 50 8B 08 FF 51 0C">().plus(1).deref<2>());
