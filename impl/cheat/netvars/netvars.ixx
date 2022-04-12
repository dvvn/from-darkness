module;

#include <string_view>

export module cheat.netvars;
export import :core;
import nstd.mem.address;
import nstd.text.chars_cache;

using nstd::text::chars_cache;
using nstd::mem::basic_address;

using _Address = nstd::mem::basic_address<const void>;

export namespace cheat::netvars
{
	_Address apply_offset(const void* thisptr, const size_t offset) noexcept;

	template<chars_cache Table, chars_cache Item>
	_Address apply_offset(const void* thisptr) noexcept
	{
		static const auto offset = get_offset(Table.view( ), Item.view( ));
		return apply_offset(thisptr, offset);
	}
}
