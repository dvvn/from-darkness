module;

#include <imgui_internal.h>
#include <d3d9.h>

export module cheat.gui:effects;

export namespace cheat::gui::effects
{
	//TODO: use service here
	//initialize after context

	void init(IDirect3DDevice9* renderer);

	void perform_blur(ImDrawList* drawList, float alpha) noexcept;

	void new_frame( ) noexcept;
	void invalidate_objects( ) noexcept;

	bool is_applicable(const ImGuiWindow* wnd);
}