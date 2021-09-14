#include "tab_bar.h"

#include "selectable.h"
#include "window.h"

#include "cheat/gui/tools/push style var.h"
#include "cheat/gui/tools/string wrapper.h"

#include <nstd/runtime assert.h>

#include <imgui_internal.h>

#include <algorithm>
#include <numeric>
#include <ranges>
#include <vector>
#include <functional>

using namespace cheat::gui;
using namespace widgets;
using namespace objects;
using namespace tools;

//static ImVec2 _Chars_to_size(size_t count)
//{
//	const auto& chr_size = _Get_char_size( );
//	//const auto& style    = ImGui::GetStyle( );
//
//	//const auto frame_padding = style.FramePadding.x * 2.f;
//
//	return {chr_size.x * count, chr_size.y};
//}

//template <typename T>
//class delayed_value
//{
//public:
//	template <typename Q>
//		requires(std::constructible_from<T, Q>)
//	delayed_value(Q&& temp_val)
//	{
//		value_ = T(std::forward<Q>(temp_val));
//	}
//
//	const std::optional<T>& begin_update( )
//	{
//		return value_temp_;
//	}
//
//	bool begin_update(const T& target_val)
//	{
//		return value_temp_.has_value( ) && *value_temp_ == target_val;
//	}
//
//	void end_update( )
//	{
//		value_ = *value_temp_;
//		value_temp_.reset( );
//	}
//
//	template <typename Q>
//		requires(std::constructible_from<T, Q>)
//	T& operator=(Q&& temp_val)
//	{
//		return value_temp_.emplace(std::forward<Q>(temp_val));
//	}
//
//	const T& get( ) const { return value_; }
//
//private:
//	T                value_;
//	std::optional<T> value_temp_;
//};
//
//template <typename Q>
//delayed_value(Q&&) -> delayed_value<std::remove_cvref_t<Q>>;

enum class directions :uint8_t
{
	UNSET
  , HORISONTAL
  , VERTICAL
};

enum class size_modes :uint8_t
{
	UNSET
  , STATIC
  , AUTO
};

struct tab_bar::impl
{
	child_frame_window        wnd;
	std::vector<tab_bar_item> items;

	size_modes size_mode = size_modes::UNSET;
	directions dir       = directions::UNSET;
};

tab_bar::tab_bar( )
{
	impl_ = std::make_unique<impl>( );
}

tab_bar::~tab_bar( ) = default;

// ReSharper disable once CppMemberFunctionMayBeConst
tab_bar_item* tab_bar::find_tab(perfect_string&& title)
{
	for (auto& item: impl_->items)
	{
		if (item.get_label( ) == title)
			return std::addressof(item);
	}

	return nullptr;
}

// ReSharper disable once CppMemberFunctionMayBeConst
tab_bar_item& tab_bar::add_tab(string_wrapper&& title)
{
	runtime_assert(find_tab(title) == nullptr);
	auto& items = impl_->items;

	auto& added = items.emplace_back( );
	added.set_label(std::move(title));

	if (items.size( ) == 1)
		items.front( ).select(callback_data(std::addressof(added), added.get_id( )));

	return added;
}

// ReSharper disable once CppMemberFunctionMayBeConst
tab_bar_item* tab_bar::get_selected( )
{
	for (auto& item: impl_->items)
	{
		if (item.selected( ))
			return std::addressof(item);
	}

	runtime_assert("No tabs selected");
	return nullptr;
}

// ReSharper disable once CppMemberFunctionMayBeConst
tab_bar_item* tab_bar::begin( )
{
	return impl_->items._Unchecked_begin( );
}

// ReSharper disable once CppMemberFunctionMayBeConst
tab_bar_item* tab_bar::end( )
{
	return impl_->items._Unchecked_end( );
}

// ReSharper disable once CppMemberFunctionMayBeConst
void tab_bar::sort(const sort_pred& pred)
{
	std::ranges::sort(impl_->items, std::ref(pred), &tab_bar_item::get_label);
}

size_t tab_bar::size( ) const
{
	return impl_->items.size( );
}

bool tab_bar::empty( ) const
{
	return impl_->items.size( );
}

// ReSharper disable once CppMemberFunctionMayBeConst
void tab_bar::make_size_static( )
{
	impl_->size_mode = size_modes::STATIC;
}

// ReSharper disable once CppMemberFunctionMayBeConst
void tab_bar::make_size_auto( )
{
	impl_->size_mode = size_modes::AUTO;
}

// ReSharper disable once CppMemberFunctionMayBeConst
void tab_bar::make_horisontal( )
{
	impl_->dir = directions::HORISONTAL;
}

// ReSharper disable once CppMemberFunctionMayBeConst
void tab_bar::make_vertical( )
{
	impl_->dir = directions::VERTICAL;
}

bool tab_bar::is_horisontal( ) const
{
	return impl_->dir == directions::HORISONTAL;
}

bool tab_bar::is_vertical( ) const
{
	return impl_->dir == directions::VERTICAL;
}


void tab_bar::render( )
{
	auto& [_Wnd, _Items, _Size_mode, _Direction] = *impl_;
	runtime_assert(!_Items.empty( ));

	// ReSharper disable once CppTooWideScopeInitStatement
	const auto wnd_size = [&]
	{
		const auto& style = ImGui::GetStyle( );

		//const auto indent_headers = style.ItemInnerSpacing.x;
		//const auto indent_page = style./*ItemInnerSpacing*/ItemSpacing.x; //ImGui use ItemSpacing by default

		const auto frame_padding = style.FramePadding * 2.f;

		const auto size_transform_helper = [&]<typename T>(T&& xy)
		{
			return _Items
				   | std::views::transform(&tab_bar_item::label_size)
				   | std::views::transform(/*&ImVec2::x*/xy);
		};

		ImVec2 size;
		if (_Direction == directions::HORISONTAL)
		{
			const auto reserve_x = [&]
			{
				const auto sizes = size_transform_helper(&ImVec2::x);
				return std::accumulate(sizes.begin( ), sizes.end( ), 0.f);
			};
			const auto reserve_y = [&]
			{
				const auto sizes = size_transform_helper(&ImVec2::y);
				return *std::ranges::max_element(sizes);
			};

			size.x = frame_padding.x +                           //space before and after
					 /*indent_headers +*/                        //to indent first selectable
					 reserve_x( ) +                              //reserve width for all strings / whole data
					 style.ItemSpacing.x * (_Items.size( ) - 1); //space between all headers

			size.y = frame_padding.y + //space before and after
					 reserve_y( );     //word height
		}
		else if (_Direction == directions::VERTICAL)
		{
			const auto reserve_x = [&]
			{
				const auto sizes = size_transform_helper(&ImVec2::x);
				return *std::ranges::max_element(sizes);
			};
			const auto reserve_y = [&]
			{
				const auto sizes = size_transform_helper(&ImVec2::y);
				return std::accumulate(sizes.begin( ), sizes.end( ), 0.f);
			};

			size.x = frame_padding.x + //space before and after
					 reserve_x( );     //reserve width for longest string

			size.y = frame_padding.y +                           //space before and after
					 reserve_y( ) +                              //all strings height						                            
					 style.ItemSpacing.y * (_Items.size( ) - 1); //space between all string
		}
		else
			runtime_assert("Unknown direction");

		return size;
	}( );

	//@note: render border manually!
	if (_Wnd.begin(wnd_size, false, ImGuiWindowFlags_None))
	{
		//@todo: move to global style
		const auto temp_center = push_style_var(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));

		const auto want_sameline = _Direction == directions::HORISONTAL;
		const auto items_count   = _Items.size( );
		const auto last_item_idx = items_count - 1;

		/*const auto item_static_size = [&]( )-> std::optional<ImVec2>
		{
			if (_Size_mode != size_modes::STATIC)
				return { };

			float max_x = 0;
			float max_y = 0;

			for (auto& i: _Items | std::views::transform(&item::label_size))
			{
				if (max_x < i.x)
					max_x = i.x;
				if (max_y < i.y)
					max_y = i.y;
			}

			return ImVec2(max_x, max_y);
		}( );*/

		for (size_t i = 0; i < items_count; i++)
		{
			auto& item = _Items[i];
			item.render(/*item_static_size*/);
			if (want_sameline && i < last_item_idx)
				ImGui::SameLine( );
		}
	}
	_Wnd.end( );
}


