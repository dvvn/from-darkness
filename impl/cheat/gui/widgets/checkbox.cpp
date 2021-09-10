#include "checkbox.h"

#include "cheat/gui/tools/string wrapper.h"

using namespace cheat::gui::widgets;

struct checkbox::data: tools::string_wrapper
{
};

checkbox::checkbox( )
{
	data_ = std::make_unique<data>( );
}

checkbox::~checkbox( ) = default;

void checkbox::render( )
{
#if CHEAT_GUI_HAS_IMGUI_STRV
	ImGui::TextUnformatted(*data_);
#else
	const auto mb = data_->multibyte( );
	ImGui::TextUnformatted(mb._Unchecked_begin( ), mb._Unchecked_end( ));
#endif

	ImGui::Checkbox()
}

// ReSharper disable once CppMemberFunctionMayBeConst
void checkbox::set(tools::string_wrapper&& checkbox)
{
	static_cast<tools::string_wrapper&>(*data_) = std::move(checkbox);
}
