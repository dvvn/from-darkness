#include "gui/menu/item_id.h"

#include <imgui_internal.h>

namespace ImGui::inline ex
{
// ReSharper disable once CppInconsistentNaming
ImGuiID GetID(int const n)
{
    return GImGui->CurrentWindow->GetID(n);
}

// ReSharper disable once CppInconsistentNaming
ImGuiID GetID(char const* first, char const* last)
{
    return GImGui->CurrentWindow->GetID(first, last);
}
}