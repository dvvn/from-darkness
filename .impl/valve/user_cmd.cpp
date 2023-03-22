#include <fd/valve/user_cmd.h>

#include <cstring>

using namespace fd::valve;

user_cmd::user_cmd()
{
    std::memset(this, 0, sizeof(user_cmd));
}
