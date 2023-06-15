#pragma once
#include <fd/abstract_interface.h>

namespace fd::valve
{
union engine
{
    FD_ABSTRACT_INTERFACE(engine);
    abstract_function<105, char const *> product_version_string;
};
}