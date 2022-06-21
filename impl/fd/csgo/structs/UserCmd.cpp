module;

#include <cstring>

module fd.csgo.structs.UserCmd;

using namespace fd::csgo;

CUserCmd::CUserCmd()
{
    std::memset(this, 0, sizeof(CUserCmd));
}
