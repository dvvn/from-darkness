#pragma once

#include "cheat/gui/tools/cached_text.h"

#include <nstd/mem/backup.h>
#include <nstd/smooth_value.h>

struct ImVec2;
using ImGuiID = unsigned int;
using ImGuiWindowFlags = int;

namespace cheat::gui::widgets
{
	class end_token
	{
	public:
		end_token( );

		end_token(const end_token& other)            = delete;
		end_token& operator=(const end_token& other) = delete;
		end_token(end_token&& other) noexcept;
		end_token& operator=(end_token&& other) noexcept;

		void set(uint8_t val);
		uint8_t release( );

		bool unset( ) const;

		bool operator!( ) const;
		bool operator==(bool val) const;
		bool operator!=(bool val) const;
		uint8_t value( ) const;

	private:
		uint8_t value_;
	};

	struct window_end_token : end_token
	{
	};

	class window_end_token_ex : public end_token
	{
	public:
		window_end_token_ex( );

		~window_end_token_ex( );
		window_end_token_ex(window_end_token_ex&& other) noexcept            = default;
		window_end_token_ex& operator=(window_end_token_ex&& other) noexcept = default;

	private:
		nstd::mem::backup<float> global_alpha_backup_;
	};

	struct window_title final : tools::cached_text
	{
		nstd::unistring<char> label_legacy;
		bool render_manually;

	protected:
		void on_update(update_flags flags) override;
	};

	window_end_token window2(const window_title& title, bool* open = nullptr, ImGuiWindowFlags flags = 0);

	struct window_wrapped
	{
		using show_anim_type = nstd::smooth_value_linear<float>;

		show_anim_type show_anim;
		ImGuiWindowFlags flags = 0;

		window_title title;

		void set(bool value);
		void toggle( );

		bool visible( ) const;
		bool show_next_tick( ) const;
		bool updating( ) const;

		window_end_token_ex operator()(bool close_button = false);

	private:
		ImGuiWindowFlags temp_flags_ = 0;
		bool show_                   = 0;
		bool show_wished_            = 0;
	};
}
