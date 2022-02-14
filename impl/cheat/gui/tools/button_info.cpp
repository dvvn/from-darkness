#include "button_info.h"

//#include "cheat/core/console.h"
#include <imgui_internal.h>

#include <vector>

using namespace cheat::gui;
using namespace tools;

button_state tools::button_behavior(const ImRect& bb, ImGuiID id, bool unreachable, ImGuiButtonFlags flags)
{
	static std::vector<std::pair<ImGuiID, button_state>> infos;

	bool hovered, held, pressed;
	if (!unreachable)
		pressed = ImGui::ButtonBehavior(bb, id, std::addressof(hovered), std::addressof(held), flags);
	else
		hovered = held = pressed = false;

	auto& state = [&]( )-> button_state&
	{
		for (auto& [id_stored, state]: infos)
		{
			if (id_stored == id)
				return state;
		}
		return infos.emplace_back(std::make_pair(id, button_state::IDLE)).second;
	}( );

	const auto mark_active = [&](button_state normal, button_state active)
	{
		if (state != active)
			state = state == normal ? active : normal;
	};

	/*auto old_state = state;*/

	if (pressed)
		state = button_state::PRESSED;
	else if (held)
		mark_active(button_state::HELD, button_state::HELD_ACTIVE);
	else if (hovered)
		mark_active(button_state::HOVERED, button_state::HOVERED_ACTIVE);
	else if (state != button_state::IDLE && state != button_state::INACTIVE)
		state = button_state::INACTIVE;
	else
		state = button_state::IDLE;

#if 0
	if (state != old_state)
	{
		constexpr auto to_string = [](button_state e)->std::string_view
		{
			switch (e)
			{
			case button_state::UNKNOWN: return "UNKNOWN";
			case button_state::IDLE: return "IDLE";
			case button_state::INACTIVE: return "INACTIVE";
			case button_state::HOVERED: return "HOVERED";
			case button_state::HOVERED_ACTIVE: return "HOVERED_ACTIVE";
			case button_state::HELD: return "HELD";
			case button_state::HELD_ACTIVE: return "HELD_ACTIVE";
			case button_state::PRESSED: return "PRESSED";
			default: throw;
			}
		};

		CHEAT_CONSOLE_LOG(std::format("{}: {} -> {}", id, to_string(old_state), to_string(state)));
	}
#endif

	return state;
}
