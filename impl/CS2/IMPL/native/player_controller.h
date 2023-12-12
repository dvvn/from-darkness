#pragma once

#include "native/base_entity.h"

namespace fd::native::inline cs2
{
class base_player_controller : public base_entity
{
  public:
    //SCHEMA(CHandle<C_CSPlayerPawnBase>, m_hPawn, "CBasePlayerController", "m_hPawn");
};

class cs_player_controller : public base_player_controller
{
  public:
    //SCHEMA(bool, m_bPawnIsAlive, "CCSPlayerController", "m_bPawnIsAlive");
    //SCHEMA(char const*, m_sSanitizedPlayerName, "CCSPlayerController", "m_sSanitizedPlayerName");
};
} // namespace fd::native::inline cs2