#include "info.h"

#include <robin_hood.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <string_view>

using namespace cheat;
using namespace gui;

const ImVec2& tools::_Get_char_size( )
{
	static robin_hood::unordered_map<size_t, ImVec2> cache;

	struct hash_data
	{
		ImFont* font;
		float   font_size;

		size_t hash( ) const
		{
			return std::_Hash_array_representation(reinterpret_cast<const uint8_t*>(this), sizeof(hash_data));
		}
	};

	const hash_data data = {ImGui::GetFont( ), ImGui::GetFontSize( )};

	auto& val = cache[data.hash( )];
	if (static auto def = ImVec2( ); val.x == def.x && val.y == def.y)
	{
		constexpr auto dummy_text = std::string_view("W");

		const auto& g = *GImGui;

		const auto font      = g.Font ? g.Font : ImGui::GetDefaultFont( );
		const auto font_size = [&]
		{
			if (g.CurrentWindow)
				return g.FontSize;

			

			return font->FontSize;
		}( );

		val = font->CalcTextSizeA(font_size, FLT_MAX, 0.f, dummy_text._Unchecked_begin( ), dummy_text._Unchecked_end( ), nullptr);

		//val                       = ImGui::CalcTextSize(dummy_text._Unchecked_begin( ), dummy_text._Unchecked_end( ));
	}

	return val;
}
