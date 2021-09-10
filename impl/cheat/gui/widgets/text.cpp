#include "text.h"

#include "cheat/gui/tools/string wrapper.h"

using namespace cheat::gui::widgets;

//struct text::data: tools::string_wrapper
//{
//};
//
text::text( )
{
	//data_ = std::make_unique<data>( );
}

text::~text( ) = default;

void text::render( )
{
	auto& l = this->get_label( );

	//todo: own imgui::text() function

#if CHEAT_GUI_HAS_IMGUI_STRV
	ImGui::TextUnformatted(l);
#else
	const auto mb = l.multibyte( );
	ImGui::TextUnformatted(mb._Unchecked_begin( ), mb._Unchecked_end( ));
#endif
}

// ReSharper disable once CppMemberFunctionMayBeConst
//void text::set_text(tools::string_wrapper&& text)
//{
//	static_cast<tools::string_wrapper&>(*data_) = std::move(text);
//}
