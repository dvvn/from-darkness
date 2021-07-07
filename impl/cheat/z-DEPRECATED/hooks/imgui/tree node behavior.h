#pragma once

#include "cheat/gui/animator.h"
#include "cheat/services/async_service.h"

namespace cheat::hooks::imgui
{


	class tree_node_behavior final: public services::async_service_shared<tree_node_behavior>,
		public decltype(detect_hook_holder(ImGui::TreeNodeBehavior))
	{
	public:
		tree_node_behavior( );

	protected:
		auto Init( ) -> void override;
		auto Callback(ImGuiID id, ImGuiTreeNodeFlags flags, const char* label, const char* label_end) -> void override;

	};
}
