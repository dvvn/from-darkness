module;

#include <fd/object.h>

module fd.valve.client_entity;
import fd.rt_modules;

FD_OBJECT_IMPL(client_entity_list, 0, fd::runtime_modules::client.find_interface<"VClientEntityList">());
