#pragma once

#include <imgui.h>

namespace ImGui::inline ex
{
// ReSharper disable once CppInconsistentNaming
bool BeginTabBar(std::type_identity_t<ImGuiID> id, ImGuiTabBarFlags flags = 0);
}