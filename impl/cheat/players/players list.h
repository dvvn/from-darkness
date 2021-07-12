#pragma once
#include "cheat/core/service.h"

#include "cheat/sdk/entity/C_CSPlayer.h"

namespace cheat
{
	class player
	{
	public:
		friend class player_shared_obj;

		player(csgo::C_CSPlayer* owner);

		bool update_simtime( );
		void update_animations( );

		csgo::C_CSPlayer* owner( ) const;

	private:
		utl::Vector      origin__,   abs_origin__;
		utl::QAngle      rotation__, abs_rotation__;
		utl::Vector      mins__,     maxs__;
		float            sim_time__;
		utl::matrix3x4_t coordinate_frame__;

		csgo::C_CSPlayer* owner__;
		int               index__;
		bool              in_use__;
	};

	using player_shared = utl::shared_ptr<player>;
	class player_shared_obj: utl::noncopyable
	{
	public:
		void init(csgo::C_CSPlayer* owner);

		player_shared_obj( ) = default;
		~player_shared_obj( );
		player_shared_obj(player_shared_obj&& other) noexcept;
		void operator=(player_shared_obj&& other) noexcept;

		player_shared share( ) const;
		player*       operator->( ) const;

		bool operator !( ) const;

	private:
		player_shared pl__;
	};

	class players_list final: public service_shared<players_list, service_mode::async>
	{
	public:
		players_list( );

		void update( );
	protected:
		void        Load( ) override;
		utl::string Get_loaded_message( ) const override;
	private:
		utl::array<player_shared_obj, 65> storage__;
	};
}
