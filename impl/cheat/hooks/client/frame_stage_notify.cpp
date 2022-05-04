module;

//#include "cheat/players/player_includes.h"

#include <string_view>

module cheat.hooks.client.frame_stage_notify;
//import cheat.players;
import cheat.csgo.interfaces.BaseClient;
import cheat.hooks.hook;

using namespace cheat;
using namespace csgo;
using namespace hooks;
using namespace client;

struct frame_stage_notify_impl final : frame_stage_notify, hook, hook_instance_member<frame_stage_notify_impl>
{
	frame_stage_notify_impl( )
	{
		entry_type entry;
		entry.set_target_method({&nstd::instance_of<IBaseClientDLL*>, 32});
		entry.set_replace_method(&frame_stage_notify_impl::callback);

		this->init(std::move(entry));
	}

	void callback(ClientFrameStage_t stage) const noexcept
	{
		switch(stage)
		{
			case FRAME_UNDEFINED: break;
			case FRAME_START: break;
			case FRAME_NET_UPDATE_START: break;
			case FRAME_NET_UPDATE_POSTDATAUPDATE_START: break;
			case FRAME_NET_UPDATE_POSTDATAUPDATE_END:
				//players::update( ); //todo: move to createmove
				break;
			case FRAME_NET_UPDATE_END: break;
			case FRAME_RENDER_START: break;
			case FRAME_RENDER_END: break;
			default:
				//runtime_assert("Unknown frame stage detectetd!");
				break;
		}

		call_original(stage);
	}
};

std::string_view frame_stage_notify::class_name( ) const noexcept
{
	return "hooks::client";
}

std::string_view frame_stage_notify::function_name( ) const noexcept
{
	return "frame_stage_notify";
}

template<>
template<>
nstd::one_instance_getter<frame_stage_notify*>::one_instance_getter(const std::in_place_index_t<0>)
	:item_(frame_stage_notify_impl::get_ptr( ))
{
}