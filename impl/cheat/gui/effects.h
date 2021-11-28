#pragma once

struct ImGuiWindow;
struct ImDrawList;

namespace cheat::gui::effects
{
	void perform_blur(ImDrawList* drawList, float alpha) noexcept;

	void new_frame( ) noexcept;
	void invalidate_objects( ) noexcept;

	bool is_applicable(const ImGuiWindow* wnd);
}
