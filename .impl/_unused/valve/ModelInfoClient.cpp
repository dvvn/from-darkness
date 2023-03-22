module;

#include <fd/object.h>

module fd.ModelInfoClient;
import fd.rt_modules;

using namespace fd;

FD_OBJECT_ATTACH_EX(IVModelInfoClient, fd::rt_modules::engine_fn().find_interface<"VModelInfoClient">());
