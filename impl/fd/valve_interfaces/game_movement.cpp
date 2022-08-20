module;

#include <fd/object.h>

module fd.valve.game_movement;
import fd.rt_modules;

FD_OBJECT_IMPL(game_movement, fd::rt_modules::client.find_interface<"GameMovement">());
