module;

#include <imgui_internal.h>

export module cheat.hooks.imgui.PushClipRect;
import dhooks;

namespace cheat::hooks::imgui
{
	export class PushClipRect final : public dhooks::select_hook_holder<decltype(ImGui::PushClipRect)>
	{
	public:
		PushClipRect( );

	protected:
		void callback(const ImVec2& clip_rect_min, const ImVec2& clip_rect_max, bool intersect_with_current_clip_rect) override;

	private:
		bool hook( ) override;
		bool enable( ) override;
		bool disable( ) override;
	};
}
