#include "helpers.h"

#include <imgui_internal.h>

namespace ImGui::ex
{
ImGuiID GetID(std::type_identity_t<int> const n)
{
    return GImGui->CurrentWindow->GetID(n);
}
}