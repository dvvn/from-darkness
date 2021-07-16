#include "push style var.h"

using namespace cheat;
using namespace gui::tools;
using namespace utl;

push_style_var::push_style_var( ) = default;

struct ImGuiStyleVarInfo;
extern const ImGuiStyleVarInfo* GetStyleVarInfo(ImGuiStyleVar idx);

struct ImGuiStyleVarInfoExtern
{
	ImGuiDataType Type;
	ImU32 Count;
	ImU32 Offset;
	void* GetVarPtr(ImGuiStyle* style) const { return reinterpret_cast<unsigned char*>(style) + Offset; }
};

push_style_var::push_style_var(ImGuiCol idx, float val)
{
	const auto var_info = (ImGuiStyleVarInfoExtern*)GetStyleVarInfo(idx);
	BOOST_ASSERT_MSG(var_info->Type == ImGuiDataType_Float && var_info->Count == 1, "Called PushStyleVar() float variant but variable is not a float!");

	auto& var = *static_cast<float*>(var_info->GetVarPtr(&GImGui->Style));
	this->emplace<0>(memory_backup(var, val));
}

push_style_var::push_style_var(ImGuiCol idx, const ImVec2& val)
{
	const auto var_info = (ImGuiStyleVarInfoExtern*)GetStyleVarInfo(idx);
	BOOST_ASSERT_MSG(var_info->Type == ImGuiDataType_Float && var_info->Count == 2, "Called PushStyleVar() ImVec2 variant but variable is not a ImVec2!");

	auto& var = *static_cast<ImVec2*>(var_info->GetVarPtr(&GImGui->Style));
	this->emplace<1>(memory_backup(var, val));
}
