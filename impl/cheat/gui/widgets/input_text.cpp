
#if 0
#include "text.h"

#include "cheat/gui/tools/string wrapper.h"

using namespace cheat::gui::widgets;

struct text::data: tools::string_wrapper
{
};

text::text( )
{
	data_ = std::make_unique<data>( );
}

text::~text( ) = default;

void text::render( )
{
#if CHEAT_GUI_HAS_IMGUI_STRV
	ImGui::TextUnformatted(*data_);
#else
	const auto mb = data_->multibyte( );
	ImGui::TextUnformatted(mb._Unchecked_begin( ), mb._Unchecked_end( ));
#endif
}

// ReSharper disable once CppMemberFunctionMayBeConst
void text::set(tools::string_wrapper&& text)
{
	static_cast<tools::string_wrapper&>(*data_) = std::move(text);
}
#endif