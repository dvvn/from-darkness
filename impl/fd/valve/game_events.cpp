module;

#include <fd/object.h>

module fd.valve.GameEvents;
import fd.rt_modules;

using namespace fd::valve;

FD_OBJECT_ATTACH_EX(game_event_manager2, fd::rt_modules::engine_fn().find_interface<"GAMEEVENTSMANAGER">());
