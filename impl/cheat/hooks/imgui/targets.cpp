#include "cheat/core/console.h"
#include "cheat/core/services_loader.h"
#include "cheat/gui/imgui_context.h"
#include "cheat/gui/effects.h"
#include "cheat/hooks/base.h"

#include <imgui_internal.h>

#include <cppcoro/task.hpp>

struct PushClipRect_callback_impl : cheat::service<PushClipRect_callback_impl>
								  , decltype(dhooks::_Detect_hook_holder(ImGui::PushClipRect))
{
	PushClipRect_callback_impl( )
	{
		this->add_dependency(cheat::gui::imgui_context::get( ));
	}

	void* get_target_method( ) const override
	{
		//return dhooks::_Pointer_to_class_method(&ImDrawList::_ResetForNewFrame);
		return ImGui::PushClipRect;
	}

	void callback(const ImVec2& clip_rect_min, const ImVec2& clip_rect_max, bool intersect_with_current_clip_rect) override
	{
		using namespace cheat::gui;

		const auto wnd = ImGui::GetCurrentWindowRead( );

		//see ImGui::Begin
		//rendering starts after this line
		if (wnd->DrawList->CmdBuffer.Size != 1 || wnd->DrawList->CmdBuffer[0].ElemCount != 0)
			return;

		//todo: remove fallback window
		if (wnd->IsFallbackWindow)
			return;
		if (!effects::is_applicable(wnd))
			return;

		this->call_original_and_store_result(clip_rect_min, clip_rect_max, intersect_with_current_clip_rect);

		const auto dlist = wnd->DrawList;
		const auto& rect = wnd->OuterRectClipped;
		dlist->PushClipRect(rect.Min, rect.Max);
		effects::perform_blur(dlist, ImGui::GetStyle( ).Alpha);
		dlist->PopClipRect( );
	}

protected:
	load_result load_impl( ) noexcept override
	{
		CHEAT_LOAD_HOOK_PROXY
	}
};

CHEAT_SERVICE_SHARE(PushClipRect_callback);

CHEAT_SERVICE_REGISTER(PushClipRect_callback);
