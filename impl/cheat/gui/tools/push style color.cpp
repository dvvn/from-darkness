#include "push style color.h"

using namespace cheat;
using namespace gui::tools;

push_style_color::push_style_color(ImGuiCol idx, const ImVec4& col)
	: memory_backup<ImVec4>(ImGui::GetStyle( ).Colors[idx], col)
{
}

push_style_color::push_style_color(ImGuiCol idx, ImU32 col)
	: push_style_color(idx, ImGui::ColorConvertU32ToFloat4(col))
{
}

static ImVec4 _Scale_alpha_color(ImGuiCol idx, float scale)
{
	auto copy = ImGui::GetStyle( ).Colors[idx];
	copy.w *= scale;
	return copy;
}

push_style_color::push_style_color(ImGuiCol idx, float alpha_scale)
	: push_style_color(idx, _Scale_alpha_color(idx, alpha_scale))
{
}
