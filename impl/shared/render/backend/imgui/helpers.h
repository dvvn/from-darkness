#pragma once

#include <imgui.h>

namespace ImGui::inline ex
{
// ReSharper disable once CppInconsistentNaming
ImGuiID GetID(std::type_identity_t<int> n);
}