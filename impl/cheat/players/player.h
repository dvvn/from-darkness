#pragma once
#include "tick record.h"

#include "cheat/sdk/entity/C_BasePlayer.h"

#include <string>
#include <veque.hpp>

#include <memory>
#include <span>

namespace cheat
{
	namespace csgo
	{
		// ReSharper disable once CppInconsistentNaming
		class C_CSPlayer;
	}

#define  CHEAT_PLAYER_PROP_FN_NAME(_NAME_) handle_##_NAME_

#define CHEAT_PLAYER_PROP_H_IMPL(_VALT_,_NAME_,_DEF_,_RETT_) \
	_VALT_ _NAME_ = _DEF_;\
	_RETT_ CHEAT_PLAYER_PROP_FN_NAME(_NAME_)(_VALT_ _NAME_##_new);

	//#define CHEAT_PLAYER_PROP_NAME_AS_TYPE0(_NAME_,_TYPE_,...) _TYPE_
	//#define CHEAT_PLAYER_PROP_NAME_AS_TYPE(_NAME_, ...) CHEAT_PLAYER_PROP_NAME_AS_TYPE0(_NAME_,##__VA_ARGS__,_NAME_)

#define CHEAT_PLAYER_PROP_H(_NAME_, _DEF_) CHEAT_PLAYER_PROP_H_IMPL(decltype(_DEF_),_NAME_,_DEF_,void)

#define CHEAT_PLAYER_PROP_FN(_NAME_) player::CHEAT_PLAYER_PROP_FN_NAME(_NAME_)

#define CHEAT_PLAYER_PROP_CPP(_NAME_) \
	auto CHEAT_PLAYER_PROP_FN(_NAME_)(decltype(_NAME_) _NAME_##_new)\
	->\
	std::invoke_result_t<decltype(&CHEAT_PLAYER_PROP_FN(_NAME_)),player*,decltype(_NAME_)>

#define CHEAT_PLAYER_PROP_FN_INVOKE(_NAME_,...)\
	std::invoke(&CHEAT_PLAYER_PROP_FN(_NAME_),this,decltype(_NAME_)(__VA_ARGS__));

	class player
	{
		template <typename T>
		// ReSharper disable once CppInconsistentNaming
		inline static T* _Defptr = nullptr;

		template <std::default_initializable T>
		// ReSharper disable once CppInconsistentNaming
		inline static T _Def = T( );

	public:
		struct team_info
		{
			constexpr team_info() = default;
			team_info(csgo::m_iTeamNum_t val);
			team_info(std::underlying_type_t<csgo::m_iTeamNum_t> val);

			csgo::m_iTeamNum_t value = csgo::m_iTeamNum_t::UNKNOWN;
			bool enemy               = false;
			bool ghost               = true; //dead,spectator

			constexpr bool operator==(const team_info&) const = default;
		};

		void update(int index);

		CHEAT_PLAYER_PROP_H(entptr, _Defptr<csgo::C_CSPlayer>);
		CHEAT_PLAYER_PROP_H(simtime, -1.f)
		CHEAT_PLAYER_PROP_H(team, _Def<team_info>)
		CHEAT_PLAYER_PROP_H(health, -1)
		CHEAT_PLAYER_PROP_H(dormant, true)

		bool local = false;

		struct ticks_info
		{
			size_t prev    = -1;
			size_t current = -1;

			void set(size_t curr)
			{
				prev    = current;
				current = curr;
			}
		};

		enum class update_state:uint8_t
		{
			IDLE
		  , SILENT
		  , NORMAL
		};

		struct
		{
			ticks_info server; //clock based
			ticks_info client; //simtime based
			update_state updated = update_state::IDLE;
		} tick;

		//--

		veque::veque<tick_record_shared> ticks;
		std::span<const tick_record_shared> ticks_window;
		static size_t max_ticks_count();
	};

	using player_shared = std::shared_ptr<player>;
}
