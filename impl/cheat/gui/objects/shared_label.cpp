#include "shared_label.h"

#include "cheat/gui/imgui context.h"
#include "cheat/gui/tools/string wrapper.h"

#include "nstd/runtime assert.h"

#include <robin_hood.h>

#include <imgui_internal.h>

#include <string>

using namespace cheat::gui::objects;
using cheat::gui::tools::string_wrapper;

struct size_obj
{
	using level3 = robin_hood::unordered_map
	<
		float  //font size;
	  , ImVec2 //label size
	>;

	using level2 = robin_hood::unordered_map
	<
		ImFont* //font ptr
	  , level3
	>;

	using level1 = robin_hood::unordered_map
	<
		size_t //label hash
	  , level2
	>;

	level1 data;

	const ImVec2& get_size(const string_wrapper& label)
	{
		const auto& g = cheat::gui::imgui_context::get_ptr( )->get( );
		runtime_assert(g.Font != nullptr);
		runtime_assert(g.FontSize > 0);

		const auto hash = [&]
		{
			const auto str = label.raw( );
			return robin_hood::hash_bytes(str._Unchecked_begin( ), str.size( ));
		}( );

		auto&  storage           = data[hash][g.Font];
		auto&& [holder, created] = storage.try_emplace(g.FontSize);
		if (created)
		{
			const auto strmb = label.multibyte( );
			holder->second   = g.Font->CalcTextSizeA(g.FontSize, FLT_MAX, 0.f, strmb._Unchecked_begin( ), strmb._Unchecked_end( ), nullptr);
		}

		return holder->second;
	}
};

//struct abstract_label::impl
//{
//	size_obj size;
//
//	void update_size(const string_wrapper& label, bool force)
//	{
//	}
//};

abstract_label::abstract_label( )
{
	//impl_ = std::make_unique<impl>( );
}

abstract_label::~abstract_label( ) = default;

const string_wrapper& abstract_label::get_label( ) const
{
	return const_cast<abstract_label*>(this)->get_label_impl( );
}

void abstract_label::set_label(string_wrapper&& new_label)
{
	get_label_impl( ) = (std::move(new_label));
}

const ImVec2& abstract_label::get_size( ) const
{
	return nstd::one_instance<size_obj>::get_ptr( )->get_size(this->get_label( ));
}

non_abstract_label::non_abstract_label(string_wrapper&& label)
{
	label_ = std::make_unique<string_wrapper>(std::move(label));
}

non_abstract_label::~non_abstract_label( )
{
}

string_wrapper& non_abstract_label::get_label_impl( )
{
	return *label_;
}

template <typename T>
static auto _Make_shared_label(const T* ptr, size_t size)
{
	return std::make_shared<non_abstract_label>(string_wrapper(std::basic_string<T>(ptr, std::next(ptr, size))));
}

std::shared_ptr<non_abstract_label> cheat::gui::objects::operator ""_shl(const char* ptr, size_t size)
{
	return _Make_shared_label(ptr, size);
}

std::shared_ptr<non_abstract_label> cheat::gui::objects::operator ""_shl(const wchar_t* ptr, size_t size)
{
	return _Make_shared_label(ptr, size);
}
