module;

#include <cstring>

module cheat.csgo.interfaces.UserCmd;

using namespace cheat::csgo;

CUserCmd::CUserCmd( )
{
	std::memset(this, 0, sizeof(CUserCmd));
}