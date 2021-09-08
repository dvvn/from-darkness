#include "tab_bar.h"

#include "selectable.h"
#include "window.h"

#include "cheat/gui/tools/info.h"
#include "cheat/gui/tools/push style var.h"
#include "cheat/gui/objects/shared_label.h"

#include <nstd/runtime assert.h>

#include <imgui_internal.h>

#include <algorithm>
#include <numeric>
#include <ranges>
#include <vector>

using namespace cheat::gui;
using namespace widgets;
using namespace objects;
using namespace tools;

static ImVec2 _Chars_to_size(size_t count)
{
	const auto& chr_size = _Get_char_size( );
	//const auto& style    = ImGui::GetStyle( );

	//const auto frame_padding = style.FramePadding.x * 2.f;

	return {chr_size.x * count, chr_size.y};
}

class tab_bar::item: selectable
{
public:
	item(const shared_label& label/*, std::optional<ImVec2> size = { }*/)
	{
		label_ = label;
		//size.has_value( ) ? set_size(*size) : set_size_auto( );
	}

	using selectable::select;
	using selectable::deselect;
	using selectable::toggle;
	using selectable::selected;

private:
	void update_label_hash( )
	{
		if (!label_hash_.has_value( ))
			return;

		const auto& label = label_->label( );
		const auto  hash  = std::invoke(std::hash<std::remove_cvref_t<decltype(label)>>( ), label);

		auto& old_hash = *label_hash_;
		if (old_hash == hash)
			return;

		old_hash = hash;
		on_label_changed( );
	}

public:
	bool render( )
	{
		this->update_label_hash( );

		return std::invoke(*static_cast<selectable*>(this), this->label( ), ImGuiSelectableFlags_None, size_);
	}

	const ImVec2& size( ) const { return size_; }

	void set_size(size_t chars_count)
	{
		const auto size = _Chars_to_size(chars_count);
		this->set_size(size);
	}

	void set_size(const ImVec2& size)
	{
		label_hash_.reset( );
		size_ = size;
	}

	void set_size_auto( )
	{
		if (label_hash_.has_value( ))
			return;

		label_hash_.emplace<size_t>(0);
		update_label_hash( );
	}

	const string_wrapper& label( ) const
	{
		return label_->label( );
	}

private:
	shared_label          label_;
	std::optional<size_t> label_hash_;

	ImVec2 size_;

	void on_label_changed( )
	{
		size_ = _Chars_to_size(this->label( ).raw( ).size( ));
	}
};

enum class directions :uint8_t
{
	UNSET,
	HORISONTAL,
	VERTICAL
};

enum class size_modes :uint8_t
{
	UNSET,
	STATIC,
	AUTO
};

template <typename T>
class delayed_value
{
public:
	template <typename Q>
		requires(std::constructible_from<T, Q>)
	delayed_value(Q&& temp_val)
	{
		value_ = T(std::forward<Q>(temp_val));
	}

	const std::optional<T>& begin_update( )
	{
		return value_temp_;
	}

	bool begin_update(const T& target_val)
	{
		return value_temp_.has_value( ) && *value_temp_ == target_val;
	}

	void end_update( )
	{
		value_ = *value_temp_;
		value_temp_.reset( );
	}

	template <typename Q>
		requires(std::constructible_from<T, Q>)
	T& operator=(Q&& temp_val)
	{
		return value_temp_.emplace(std::forward<Q>(temp_val));
	}

	const T& get( ) const { return value_; }

private:
	T                value_;
	std::optional<T> value_temp_;
};

template <typename Q>
delayed_value(Q&&) -> delayed_value<std::remove_cvref_t<Q>>;

struct tab_bar::impl
{
	child_frame_window wnd;
	std::vector<item>  items;

	delayed_value<size_modes> size_mode = size_modes::UNSET;
	delayed_value<directions> dir       = directions::UNSET;
};

tab_bar::tab_bar( )
{
	impl_ = std::make_unique<impl>( );
}

tab_bar::~tab_bar( ) = default;

size_t tab_bar::get_index(perfect_string&& title) const
{
	const auto& items = impl_->items;
	for (size_t i = 0; i < items.size( ); i++)
	{
		if (items[i].label( ) == title)
			return i;
	}
	return static_cast<size_t>(-1);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void tab_bar::add_tab(const shared_label& title)
{
	runtime_assert(get_index(title->label( )) == static_cast<size_t>(-1));
	auto& items = impl_->items;

	items.push_back(title);
	if (items.size( ) == 1)
		items.front( ).select( );
}

size_t tab_bar::get_selected_index( ) const
{
	const auto& items = impl_->items;
	for (size_t i = 0; i < items.size( ); ++i)
	{
		if (items[i].selected( ))
			return i;
	}
	runtime_assert("No tabs selected");
	return static_cast<size_t>(-1);
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

void tab_bar::handle_size_change( )
{
	auto& size_mode = impl_->size_mode;
	auto& new_val   = size_mode.begin_update( );
	if (!new_val)
		return;

	switch (*new_val)
	{
		case size_modes::STATIC:
		{
			auto& items = impl_->items;

			const auto longest = [&]
			{
				const auto sizes = items
								   | std::views::transform(&item::label)
								   | std::views::transform(&string_wrapper::raw)
								   | std::views::transform(&std::wstring_view::size);
				return *std::ranges::max_element(sizes);
			}( );

			for (auto& i: items)
				i.set_size(longest);
			break;
		}
		case size_modes::AUTO:
		{
			std::ranges::for_each(impl_->items, &item::set_size_auto);
			break;
		}
		default:
		{
			runtime_assert("Wrong size mode!");
			break;
		}
	}

	size_mode.end_update( );
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

// ReSharper disable once CppMemberFunctionMayBeConst
void tab_bar::handle_direction_change( )
{
	auto& dir = impl_->dir;
	if (!dir.begin_update( ))
		return;

	//-----

	dir.end_update( );
}

bool tab_bar::is_horisontal( ) const
{
	return impl_->dir.get( ) == directions::HORISONTAL;
}

bool tab_bar::is_vertical( ) const
{
	return impl_->dir.get( ) == directions::VERTICAL;
}

void tab_bar::render( )
{
	this->handle_size_change( );
	this->handle_direction_change( );

	const auto& direction = impl_->dir.get( );

	const auto render_bar = [&]
	{
		auto& wnd   = impl_->wnd;
		auto& items = impl_->items;

		// ReSharper disable once CppTooWideScopeInitStatement
		const auto wnd_size = [&]
		{
			const auto& style = ImGui::GetStyle( );

			//const auto indent_headers = style.ItemInnerSpacing.x;
			//const auto indent_page = style./*ItemInnerSpacing*/ItemSpacing.x; //ImGui use ItemSpacing by default

			const auto frame_padding = style.FramePadding * 2.f;

			const auto transform_helper = [&]<typename T>(T&& xy)
			{
				return items
					   | std::views::transform(&item::size)
					   | std::views::transform(/*&ImVec2::x*/xy);
			};

			ImVec2 size;
			if (direction == directions::HORISONTAL)
			{
				const auto reserve_x = [&]
				{
					const auto sizes = transform_helper(&ImVec2::x);
					return std::accumulate(sizes.begin( ), sizes.end( ), 0.f);
				};
				const auto reserve_y = [&]
				{
					const auto sizes = transform_helper(&ImVec2::y);
					return *std::ranges::max_element(sizes);
				};

				size.x = frame_padding.x +                          //space before and after
						 /*indent_headers +*/                       //to indent first selectable
						 reserve_x( ) +                             //reserve width for all strings / whole data
						 style.ItemSpacing.x * (items.size( ) - 1); //space between all headers

				size.y = frame_padding.y + //space before and after
						 reserve_y( );     //word height
			}
			else if (direction == directions::VERTICAL)
			{
				const auto reserve_x = [&]
				{
					const auto sizes = transform_helper(&ImVec2::x);
					return *std::ranges::max_element(sizes);
				};
				const auto reserve_y = [&]
				{
					const auto sizes = transform_helper(&ImVec2::y);
					return std::accumulate(sizes.begin( ), sizes.end( ), 0.f);
				};

				size.x = frame_padding.x + //space before and after
						 reserve_x( );     //reserve width for longest string

				size.y = frame_padding.y +                          //space before and after
						 reserve_y( ) +                             //all strings height						                            
						 style.ItemSpacing.y * (items.size( ) - 1); //space between all string
			}
			else
				runtime_assert("Unknown direction");

			return size;
		}( );

		if (wnd.begin(wnd_size))
		{
			const auto temp_center = push_style_var(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));

			const auto want_sameline = direction == directions::HORISONTAL;
			const auto items_count   = items.size( );
			const auto last_item_idx = items_count - 1;
			for (size_t i = 0; i < items_count; i++)
			{
				if (auto& item = items[i]; item.render( ))
				{
					items[this->get_selected_index( )].deselect( );
					item.select( );
				}
				if (want_sameline && i < last_item_idx)
					ImGui::SameLine( );
			}
		}
		wnd.end( );
	};

	//-----

	render_bar( );
}
