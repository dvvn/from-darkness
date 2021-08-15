#include "info.h"

using namespace cheat;
using namespace gui;
using namespace utl;

const ImVec2& tools::_Get_char_size( )
{
	struct alignas(uint64_t) hash_data
	{
		ImFont* font;
		float font_size;

		uint64_t hash( ) const
		{
			static_assert(sizeof(hash_data) == sizeof(uint64_t));
			return *reinterpret_cast<const uint64_t*>(this);
		}
	};

	static nstd::unordered_map<uint64_t, ImVec2> cache;

	const hash_data data = {ImGui::GetFont( ), ImGui::GetFontSize( )};

	auto& val = cache[data.hash( )];
	if (static auto def = ImVec2( );
		val.x == def.x && val.y == def.y)
	{
		constexpr auto dummy_text = std::string_view("W");
		val = ImGui::CalcTextSize(dummy_text._Unchecked_begin( ), dummy_text._Unchecked_end( ));
	}

	return val;
}
