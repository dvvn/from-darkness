#pragma once

#include <spdlog/spdlog.h>

namespace fd::log
{
using level = spdlog::level::level_enum;
using spdlog::set_level;

using spdlog::critical;
using spdlog::debug;
using spdlog::error;
using spdlog::info;
using spdlog::trace;
using spdlog::warn;
}