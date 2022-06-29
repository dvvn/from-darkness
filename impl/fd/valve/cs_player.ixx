module;

export module fd.valve.cs_player;
export import fd.valve.base_player;

using namespace fd::valve;

struct cs_player : base_player
{
#if __has_include("C_CSPlayer_generated_h")
#include "C_CSPlayer_generated_h"
#endif

    base_animating* rag_doll();
};

export namespace fd::valve
{

    using ::cs_player;

}
