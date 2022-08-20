module;

#include <fd/object.h>

module fd.valve.GameEvents;
import fd.rt_modules;

using namespace fd::valve;

FD_OBJECT_IMPL(game_event_manager2, fd::rt_modules::engine.find_interface<"GAMEEVENTSMANAGER">());
