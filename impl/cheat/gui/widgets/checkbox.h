#pragma once
#include "selectable.h"

namespace cheat::gui::tools
{
	enum class button_state : int8_t;
}

namespace cheat::gui::widgets
{
	class checkbox : public selectable_bg
				   , public text
	{
	public:
		checkbox();
		~checkbox() override;

		checkbox(checkbox&&) noexcept;
		checkbox& operator=(checkbox&&) noexcept;

		enum state:uint8_t
		{
			STATE_IDLE
		  , STATE_SELECTED
		};

		void render() override;

		void set_check_color_modifier(std::unique_ptr<animation_property<ImVec4>>&& mod);

	protected:
		void render_check_mark(ImGuiWindow* window, const ImVec2& basic_pos, float basic_size);
		//ImGuiButtonFlags_ get_button_flags( ) const override;

	private:
		struct impl;
		std::unique_ptr<impl> impl_;
	};

	//todo: move outside
	template <typename T>
	ImGuiID make_imgui_id(const T& obj, ImGuiWindow* wnd)
	{
		static_assert(sizeof(ImGuiID) % sizeof(void*) == 0);

		const auto ptr     = std::addressof(obj);
		const auto id_temp = reinterpret_cast<uintptr_t>(ptr);

		constexpr auto arr_size = sizeof(uintptr_t) / sizeof(ImGuiID);
		using arr = std::array<ImGuiID, arr_size>;

		auto& id_arr    = reinterpret_cast<const arr&>(id_temp);
		const auto seed = wnd->IDStack.back( );

		const auto make_id = [&]<size_t ...I>(std::index_sequence<I...>)
		{
			return (id_arr[I] ^ ...) ^ seed;
		};

		const ImGuiID id = make_id(std::make_index_sequence<arr_size>( ));

		ImGui::KeepAliveID(id);
		return id;
	}

	template <typename T>
	class unconfirmed_value
	{
	public:
		unconfirmed_value(const T& value)
			: value_(value)
		{
		}

		//bool operator==(const unconfirmed_value& val) const = default;

		operator T&() { return value_; }
		operator const T&() const { return value_; }

		unconfirmed_value(const unconfirmed_value& other)            = default;
		unconfirmed_value& operator=(const unconfirmed_value& other) = default;

		T* operator&() { return std::addressof(value_); }

	private:
		T value_;
	};

	template <typename T>
	class confirmable_value : public unconfirmed_value<T>
	{
	public:
		confirmable_value(const T& value)
			: unconfirmed_value<T>(value), value_confirmed_( )
		{
			if constexpr (std::is_trivially_destructible_v<T>)
			{
				using arr = std::array<uint8_t, sizeof(T)>;
				reinterpret_cast<arr&>(value_confirmed_)[0] = ~reinterpret_cast<const arr&>(value)[0];
			}
			else
			{
				static_assert(std::_Always_false<T>,__FUNCTION__": not trivially destructible (todo)");
			}
		}

		void confirm() { value_confirmed_ = *this; }
		bool confirmed() const { return *this == value_confirmed_; }

	private:
		T value_confirmed_;
	};

	tools::button_state checkbox2(const tools::cached_text& label, confirmable_value<bool>& value
								, animation_property<ImVec4>* bg_animation    = 0
								, animation_property<ImVec4>* check_animation = 0);
}
