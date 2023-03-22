module;

#include <fd/object.h>

module fd.valve.game_movement;
import fd.rt_modules;

FD_OBJECT_ATTACH_EX(game_movement, fd::rt_modules::client_fn().find_interface<"GameMovement">());
