module;

module cheat.csgo.interfaces.Input;
import cheat.csgo.structs.UserCmd;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CInput* nstd::one_instance_getter<CInput*>::_Construct( )const
{
	return csgo_modules::client.find_signature<"B9 ? ? ? ? F3 0F 11 04 24 FF 50 10">( ).plus(1).deref<1>( );
}

constexpr auto MULTIPLAYER_BACKUP = 150;

CUserCmd* CInput::GetUserCmd(int sequence_number)
{
	return &m_pCommands[sequence_number % MULTIPLAYER_BACKUP];
}

CVerifiedUserCmd* CInput::GetVerifiedCmd(int sequence_number)
{
	return &m_pVerifiedCommands[sequence_number % MULTIPLAYER_BACKUP];
}
