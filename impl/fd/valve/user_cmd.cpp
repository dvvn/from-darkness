module;

#include <cstring>

module fd.valve.user_cmd;

user_cmd::user_cmd()
{
    std::memset(this, 0, sizeof(user_cmd));
}
