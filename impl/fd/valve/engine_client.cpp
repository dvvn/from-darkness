module;

#include <fd/object.h>

module fd.valve.engine_client;
import fd.rt_modules;

FD_OBJECT_IMPL(engine_client, 0, fd::runtime_modules::engine.find_interface<"VEngineClient">());
