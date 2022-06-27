module;

export module fd.cs_player;
export import fd.base_player;

struct cs_player : fd::base_player
{
#if __has_include("C_CSPlayer_generated_h")
#include "C_CSPlayer_generated_h"
#endif

    fd::base_animating* rag_doll();
};

export namespace fd
{

    using ::cs_player;

}
