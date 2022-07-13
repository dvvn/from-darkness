module;

#include <fd/object.h>

module fd.ModelInfoClient;
import fd.rt_modules;

using namespace fd;

FD_OBJECT_IMPL(IVModelInfoClient, fd::rt_modules::engine.find_interface<"VModelInfoClient">());
