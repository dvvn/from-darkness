#include "push style var.h"

using namespace cheat;
using namespace gui::tools;

push_style_var::push_style_var( ) = default;

struct ImGuiStyleVarInfo
{
	ImGuiDataType type;
	ImU32         count;
	ImU32         offset;
	void*         GetVarPtr(ImGuiStyle* style) const { return reinterpret_cast<uint8_t*>(style) + offset; }
};

static const ImGuiStyleVarInfo img_stylevar_info[] =
{
	{ImGuiDataType_Float, 1, (IM_OFFSETOF(ImGuiStyle, Alpha))},               // ImGuiStyleVar_Alpha
	{ImGuiDataType_Float, 2, (IM_OFFSETOF(ImGuiStyle, WindowPadding))},       // ImGuiStyleVar_WindowPadding
	{ImGuiDataType_Float, 1, (IM_OFFSETOF(ImGuiStyle, WindowRounding))},      // ImGuiStyleVar_WindowRounding
	{ImGuiDataType_Float, 1, (IM_OFFSETOF(ImGuiStyle, WindowBorderSize))},    // ImGuiStyleVar_WindowBorderSize
	{ImGuiDataType_Float, 2, (IM_OFFSETOF(ImGuiStyle, WindowMinSize))},       // ImGuiStyleVar_WindowMinSize
	{ImGuiDataType_Float, 2, (IM_OFFSETOF(ImGuiStyle, WindowTitleAlign))},    // ImGuiStyleVar_WindowTitleAlign
	{ImGuiDataType_Float, 1, (IM_OFFSETOF(ImGuiStyle, ChildRounding))},       // ImGuiStyleVar_ChildRounding
	{ImGuiDataType_Float, 1, (IM_OFFSETOF(ImGuiStyle, ChildBorderSize))},     // ImGuiStyleVar_ChildBorderSize
	{ImGuiDataType_Float, 1, (IM_OFFSETOF(ImGuiStyle, PopupRounding))},       // ImGuiStyleVar_PopupRounding
	{ImGuiDataType_Float, 1, (IM_OFFSETOF(ImGuiStyle, PopupBorderSize))},     // ImGuiStyleVar_PopupBorderSize
	{ImGuiDataType_Float, 2, (IM_OFFSETOF(ImGuiStyle, FramePadding))},        // ImGuiStyleVar_FramePadding
	{ImGuiDataType_Float, 1, (IM_OFFSETOF(ImGuiStyle, FrameRounding))},       // ImGuiStyleVar_FrameRounding
	{ImGuiDataType_Float, 1, (IM_OFFSETOF(ImGuiStyle, FrameBorderSize))},     // ImGuiStyleVar_FrameBorderSize
	{ImGuiDataType_Float, 2, (IM_OFFSETOF(ImGuiStyle, ItemSpacing))},         // ImGuiStyleVar_ItemSpacing
	{ImGuiDataType_Float, 2, (IM_OFFSETOF(ImGuiStyle, ItemInnerSpacing))},    // ImGuiStyleVar_ItemInnerSpacing
	{ImGuiDataType_Float, 1, (IM_OFFSETOF(ImGuiStyle, IndentSpacing))},       // ImGuiStyleVar_IndentSpacing
	{ImGuiDataType_Float, 2, (IM_OFFSETOF(ImGuiStyle, CellPadding))},         // ImGuiStyleVar_CellPadding
	{ImGuiDataType_Float, 1, (IM_OFFSETOF(ImGuiStyle, ScrollbarSize))},       // ImGuiStyleVar_ScrollbarSize
	{ImGuiDataType_Float, 1, (IM_OFFSETOF(ImGuiStyle, ScrollbarRounding))},   // ImGuiStyleVar_ScrollbarRounding
	{ImGuiDataType_Float, 1, (IM_OFFSETOF(ImGuiStyle, GrabMinSize))},         // ImGuiStyleVar_GrabMinSize
	{ImGuiDataType_Float, 1, (IM_OFFSETOF(ImGuiStyle, GrabRounding))},        // ImGuiStyleVar_GrabRounding
	{ImGuiDataType_Float, 1, (IM_OFFSETOF(ImGuiStyle, TabRounding))},         // ImGuiStyleVar_TabRounding
	{ImGuiDataType_Float, 2, (IM_OFFSETOF(ImGuiStyle, ButtonTextAlign))},     // ImGuiStyleVar_ButtonTextAlign
	{ImGuiDataType_Float, 2, (IM_OFFSETOF(ImGuiStyle, SelectableTextAlign))}, // ImGuiStyleVar_SelectableTextAlign
};

static const ImGuiStyleVarInfo* _Get_style_var_info(ImGuiStyleVar idx)
{
	runtime_assert(idx >= 0 && idx < ImGuiStyleVar_COUNT);
	static_assert(std::size(img_stylevar_info) == ImGuiStyleVar_COUNT);
	return &img_stylevar_info[idx];
}

push_style_var::push_style_var(ImGuiCol idx, float val)
{
	const auto var_info = _Get_style_var_info(idx);
	runtime_assert(var_info->type == ImGuiDataType_Float && var_info->count == 1, "Called PushStyleVar() float variant but variable is not a float!");

	auto& var = *static_cast<float*>(var_info->GetVarPtr(std::addressof(ImGui::GetStyle( ))));
	this->emplace<0>(nstd::memory_backup(var, val));
}

push_style_var::push_style_var(ImGuiCol idx, const ImVec2& val)
{
	const auto var_info = _Get_style_var_info(idx);
	runtime_assert(var_info->type == ImGuiDataType_Float && var_info->count == 2, "Called PushStyleVar() ImVec2 variant but variable is not a ImVec2!");

	auto& var = *static_cast<ImVec2*>(var_info->GetVarPtr(std::addressof(ImGui::GetStyle( ))));
	this->emplace<1>(nstd::memory_backup(var, val));
}
