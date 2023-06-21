﻿#pragma once
#include <fd/abstract_interface.h>

namespace fd::valve
{
// ReSharper disable once CppInconsistentNaming
class IEngineClient;

union engine
{
    FD_ABSTRACT_INTERFACE(IEngineClient);
    abstract_function<12, uint32_t> local_player_index;
    abstract_function<20, uint32_t> max_clients;
    abstract_function<26, bool> in_game;
    abstract_function<105, char const *> product_version_string;
};
}