module;

#include <fd/object.h>

module fd.valve.view_render;
import fd.rt_modules;

FD_OBJECT_ATTACH_EX(view_render, fd::rt_modules::client_fn().find_interface_sig<"8B 0D ? ? ? ? FF 75 0C 8B 45 08", 0x2, 2, "class IViewRender">());
