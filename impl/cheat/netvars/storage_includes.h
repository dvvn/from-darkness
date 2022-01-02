#pragma once

#if defined(_DEBUG) || defined(CHEAT_NETVARS_UPDATING)
#define CHEAT_NETVARS_RESOLVE_TYPE
#endif

#ifdef CHEAT_NETVARS_RESOLVE_TYPE
#include <nlohmann/json.hpp>
#include <nlohmann/ordered_map.hpp>
#else
#include <nstd/unordered_map.h>
#endif 

#include <nstd/ranges.h>
