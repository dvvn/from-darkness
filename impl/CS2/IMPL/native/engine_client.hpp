#pragma once

namespace fd::native::inline cs2
{
// Source2EngineToClient001
class engine_client
{
  public:
    bool is_in_game();
    int get_local_player();
    int get_engine_build_number();
};
}