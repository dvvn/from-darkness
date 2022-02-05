module;

#include "cheat/hooks/base_includes.h"
#include <imgui_internal.h>

export module cheat.hooks.imgui:PushClipRect;
import cheat.hooks.base;

namespace cheat::hooks::imgui
{
	export class PushClipRect final : public hook_base<PushClipRect, decltype(ImGui::PushClipRect)>
	{
	public:
		PushClipRect( );

	protected:
		void construct( ) noexcept override;
		void callback(const ImVec2& clip_rect_min, const ImVec2& clip_rect_max, bool intersect_with_current_clip_rect) override;
	};
}
