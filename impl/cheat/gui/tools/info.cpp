#include "info.h"

#include <robin_hood.h>

#include <imgui.h>

#include <string_view>

using namespace cheat;
using namespace gui;

const ImVec2& tools::_Get_char_size( )
{
	struct hash_data
	{
		ImFont* font;
		float   font_size;

		size_t hash( ) const
		{
			return robin_hood::hash_bytes(this, sizeof(hash_data));
		}
	};

	static robin_hood::unordered_map<uint64_t, ImVec2> cache;

	const hash_data data = {ImGui::GetFont( ), ImGui::GetFontSize( )};

	auto& val = cache[data.hash( )];
	if (static auto def = ImVec2( );
		val.x == def.x && val.y == def.y)
	{
		constexpr auto dummy_text = std::string_view("W");
		val                       = ImGui::CalcTextSize(dummy_text._Unchecked_begin( ), dummy_text._Unchecked_end( ));
	}

	return val;
}
