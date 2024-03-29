﻿#pragma once
#include "internal/native_interface.h"

namespace fd
{
struct native_library_info;

FD_BIND_NATIVE_INTERFACE(IEngineClient, engine);

union native_engine
{
    native_engine(native_library_info info);

    FD_NATIVE_INTERFACE(IEngineClient);
    function<12, uint32_t> local_player_index;
    function<20, uint32_t> max_clients;
    function<26, bool> in_game;
    function<105, char const *> product_version_string;
};
}