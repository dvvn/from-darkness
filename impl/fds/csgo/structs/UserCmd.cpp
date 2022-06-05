module;

#include <cstring>

module fds.csgo.structs.UserCmd;

using namespace fds::csgo;

CUserCmd::CUserCmd()
{
    std::memset(this, 0, sizeof(CUserCmd));
}
