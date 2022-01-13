#pragma once

#include "cheat/service/includes.h"

#include <nstd/format.h>
#include <nstd/ranges.h>

#include <windows.h>

#include <string>
#include <variant>
#include <sstream>
#include <functional>

#ifdef CHEAT_HAVE_CONSOLE
#define CHEAT_CONSOLE_LOG_IMPL(_HOLDER_,...) _HOLDER_->deps( ).get<console>( ).log(__VA_ARGS__)
#else
#define CHEAT_CONSOLE_LOG_IMPL(...) (void)0
#endif

#define CHEAT_CONSOLE_LOG(...) CHEAT_CONSOLE_LOG_IMPL(this,__VA_ARGS__)
#define CHEAT_CONSOLE_LOG_G(...) CHEAT_CONSOLE_LOG_IMPL(cheat::services_loader::get_ptr( ),__VA_ARGS__)


