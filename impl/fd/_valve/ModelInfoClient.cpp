module;

#include <fd/object.h>

module fd.ModelInfoClient;
import fd.rt_modules;

using namespace fd;

FD_OBJECT_IMPL(IVModelInfoClient, 0, fd::runtime_modules::engine.find_interface<"VModelInfoClient">());
