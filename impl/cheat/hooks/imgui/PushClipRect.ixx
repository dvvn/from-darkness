module;

#include "cheat/hooks/base_includes.h"
#include <imgui_internal.h>

export module cheat.hooks.imgui:PushClipRect;
import cheat.hooks.base;

export namespace cheat::hooks::imgui
{
	struct PushClipRect final : hook_base<PushClipRect, decltype(ImGui::PushClipRect)>
	{
		PushClipRect( );

	protected:
		void callback(const ImVec2& clip_rect_min, const ImVec2& clip_rect_max, bool intersect_with_current_clip_rect) override;
		void* get_target_method( ) const override;
	};
}
