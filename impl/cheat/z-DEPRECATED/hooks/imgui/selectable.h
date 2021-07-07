#pragma once

#include "cheat/services/async_service.h"

namespace cheat::hooks::imgui
{
	namespace detail
	{
		constexpr bool (*selectable_fn)(const char*, bool, ImGuiSelectableFlags, const ImVec2&) = ImGui::Selectable;
	}

	class selectable final: public services::async_service_shared<selectable>,
							public decltype(detect_hook_holder(detail::selectable_fn))
	{
	public:
		selectable( );

	protected:
		auto Init( ) -> void override;
		auto Callback(const char* label, bool selected, ImGuiSelectableFlags flags, const ImVec2& size_arg) -> void override;

	private:
		utl::unordered_map<ImGuiID, bool> selected__;
	};
}
