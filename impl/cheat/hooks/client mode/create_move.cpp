module;

#include <string_view>

module cheat.hooks.client_mode.create_move;
import cheat.hooks.hook;
import cheat.csgo.interfaces.Prediction;
import cheat.csgo.interfaces.EngineClient;
import cheat.csgo.interfaces.ClientMode;

using namespace cheat;
using namespace csgo;
using namespace hooks;
using namespace client_mode;

struct create_move_impl final : create_move, hook, hook_instance_member<create_move_impl>
{
	create_move_impl( )
	{
		entry_type entry;
		entry.set_target_method({&nstd::instance_of<ClientModeShared*>, 24});
		entry.set_replace_method(&create_move_impl::callback);

		this->init(std::move(entry));
	}

	bool callback(float input_sample_time, CUserCmd* cmd) const noexcept
	{
		const auto original_return = call_original(input_sample_time, cmd);

		// is called from CInput::ExtraMouseSample
		if(cmd->iCommandNumber == 0)
			return original_return;

		if(original_return == true)
		{
			nstd::instance_of<IPrediction*>->SetLocalViewAngles(cmd->angViewPoint);
			nstd::instance_of<IVEngineClient*>->SetViewAngles(cmd->angViewPoint);
		}

		if(/*interfaces.client_state == nullptr ||*/ nstd::instance_of<IVEngineClient*>->IsPlayingDemo( ))
			return original_return;

		//bool& send_packet = address(/*this->return_address( )*/*this->addr1).remove(4).deref(1).remove(0x1C).ref( );

		return false;
	}
};

std::string_view create_move::class_name( ) const noexcept
{
	return "hooks::client_mode";
}

std::string_view create_move::function_name( ) const noexcept
{
	return "create_move";
}

template<>
template<>
nstd::one_instance_getter<create_move*>::one_instance_getter(const std::in_place_index_t<0>)
	:item_(create_move_impl::get_ptr( ))
{
}