#pragma once
#include "UtlVector.hpp"

namespace cheat::csgo
{
	class C_BaseEntity;

	class CGlowObjectManager
	{
	public:
		static constexpr auto END_OF_FREE_LIST = -1;
		static constexpr auto ENTRY_IN_USE = -2;

		enum render_styles : int
		{
			RENDER_STYLE_DEFAULT = 0,
			RENDER_STYLE_RIMGLOW3D,
			RENDER_STYLE_EDGE_HIGHLIGHT,
			RENDER_STYLE_EDGE_HIGHLIGHT_PULSE
		};

		struct GlowObject_t
		{
			void Set(const utl::Color& glow_color, render_styles render_style = RENDER_STYLE_DEFAULT)
			// @note: styles not used cuz other styles doesnt have ignorez flag and needed to rebuild glow
			{
				this->color = {glow_color[0], glow_color[1], glow_color[2], glow_color[3]};
				this->bloom_amount = 1.0f;
				this->render_when_occluded = true;
				this->render_when_unoccluded = false;
				this->render_style = render_style;
			}

			bool IsEmpty( ) const
			{
				return next_free_slot != ENTRY_IN_USE;
			}

			int                  next_free_slot;                 // 0x00
			C_BaseEntity*        entity;                         // 0x04
			utl::array<float, 4> color;                          // 0x08
			bool                 alpha_capped_by_render_alpha;   // 0x18
			std::byte            pad0[0x3];                      // 0x19 - pack 1 bool as 4 bytes
			float                alpha_function_of_max_velocity; // 0x1C
			float                bloom_amount;                   // 0x20
			float                pulse_overdrive;                // 0x24
			bool                 render_when_occluded;           // 0x28
			bool                 render_when_unoccluded;         // 0x29
			bool                 full_bloom_render;              // 0x2A
			std::byte            pad1[0x1];                      // 0x2B  - pack 3 bool as 4 bytes
			int                  full_bloom_stencil_test_value;  // 0x2C
			render_styles        render_style;                   // 0x30
			int                  split_screen_slot;              // 0x34
			//-
		}; // Size: 0x38

		struct GlowBoxObject_t
		{
			utl::Vector position;
			utl::QAngle orientation;
			utl::Vector mins;
			utl::Vector maxs;
			float       birth_time_index;
			float       termination_time_index;
			utl::Color  color;
		};

		CUtlVector<GlowObject_t>    glow_object_definitions;
		int                         first_free_slot;
		CUtlVector<GlowBoxObject_t> glow_box_definitions;
	};
}
