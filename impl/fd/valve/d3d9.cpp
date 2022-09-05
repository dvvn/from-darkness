#include <fd/object.h>

import fd.rt_modules;

struct IDirect3DDevice9;

// @xref: "HandleLateCreation"
FD_OBJECT_IMPL(IDirect3DDevice9*, (fd::rt_modules::shaderApiDx9.find_interface_sig<"A1 ? ? ? ? 50 8B 08 FF 51 0C", 0x1, 2, IDirect3DDevice9>()));
