module;

module cheat.csgo.interfaces:Input;
import :UserCmd;

using namespace cheat::csgo;

constexpr auto MULTIPLAYER_BACKUP = 150;

CUserCmd* CInput::GetUserCmd(int sequence_number)
{
    return &m_pCommands[sequence_number % MULTIPLAYER_BACKUP];
}
 
CVerifiedUserCmd* CInput::GetVerifiedCmd(int sequence_number)
{
    return &m_pVerifiedCommands[sequence_number % MULTIPLAYER_BACKUP];
}
