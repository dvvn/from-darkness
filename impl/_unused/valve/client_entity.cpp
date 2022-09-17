module;

#include <fd/object.h>

module fd.valve.client_entity;
import fd.rt_modules;

FD_OBJECT_ATTACH_EX(client_entity_list, fd::rt_modules::client_fn().find_interface<"VClientEntityList">());
