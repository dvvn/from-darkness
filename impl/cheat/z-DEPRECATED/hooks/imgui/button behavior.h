#pragma once

#include "cheat/services/async_service.h"

namespace cheat::hooks::imgui
{
	class button_behavior final: public services::async_service_shared<button_behavior>,
								 public decltype(detect_hook_holder(ImGui::ButtonBehavior))
	{
	public:
		button_behavior( );

		using func_type = utl::function<void(bool,const ImRect&, ImGuiID, bool&, bool&, utl::bitflag<ImGuiButtonFlags_>)>;

		auto set_func(ImGuiID id, func_type&& func) -> void;
		auto reset_func( ) -> void;

	protected:
		auto Init( ) -> void override;
		auto Callback(const ImRect& bb, ImGuiID id, bool* out_hovered, bool* out_held, ImGuiButtonFlags flags) -> void override;

	private:
		utl::optional<ImGuiID> id__;
		func_type              func__;
	};
}
