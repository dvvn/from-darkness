module;

#include <fd/object.h>

module fd.valve.view_render;
import fd.rt_modules;

FD_OBJECT_IMPL(view_render, 0, fd::runtime_modules::client.find_interface_sig<"A1 ? ? ? ? B9 ? ? ? ? C7 05 ? ? ? ? ? ? ? ? FF 10">().plus(1).deref<1>());
