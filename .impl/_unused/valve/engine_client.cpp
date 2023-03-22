module;

#include <fd/object.h>

module fd.valve.engine_client;
import fd.rt_modules;

FD_OBJECT_ATTACH_EX(engine_client, fd::rt_modules::engine_fn().find_interface<"VEngineClient">());
