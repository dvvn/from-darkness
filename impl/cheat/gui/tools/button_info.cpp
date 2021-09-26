#include "button_info.h"

#include <robin_hood.h>

using namespace cheat::gui::tools;

button_state cheat::gui::tools::button_behavior(const ImRect& bb, ImGuiID id, button_flags flags)
{
	static robin_hood::unordered_map<ImGuiID, button_state> infos;

	// ReSharper disable once CppJoinDeclarationAndAssignment
	bool hovered, held, pressed;
	pressed = ImGui::ButtonBehavior(bb, id, std::addressof(hovered), std::addressof(held), flags.fi);

	button_state& state = infos[id];

	//state_last = state;

	const auto mark_active = [&](button_state normal, button_state active)
	{
		if (state != active)
			state = state == normal ? active : normal;
	};

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

	return state;
}
