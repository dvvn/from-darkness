#include "pages renderer.h"

#include "cheat/gui/tools/info.h"
#include "cheat/gui/tools/push style var.h"
#include "cheat/gui/widgets/window.h"

#include "nstd/runtime assert.h"

#include <imgui_internal.h>

#include <algorithm>
#include <numeric>
#include <ranges>
#include <span>

using namespace cheat;
using namespace gui;
using namespace tools;
using namespace objects;

#if 0

static size_t _Get_num_chars(const pages_storage_data& page)
{
	return page.name( ).raw( ).size( );
}

static auto _Sizes(std::span<pages_storage_data>&& container)
{
	return container | std::views::transform(_Get_num_chars);
}

struct selected_info
{
	pages_storage_data* ptr;
	bool                activated;
};

template <bool Sameline, class Fns>
static selected_info _Render_and_select(std::span<pages_storage_data>&& data, Fns&& size_getter)
{
	pages_storage_data* page_active = 0;

	const auto data_begin = data.begin( );
	const auto data_end   = data.end( );
	const auto pre_end    = [&]( )-> std::_Span_iterator<pages_storage_data>
	{
		if constexpr (!Sameline)
			return { };
		else
			return std::prev(data_end);
	}( );

	const auto obj_invoke = [&](pages_storage_data& obj)
	{
		const ImVec2 size = std::invoke(size_getter, obj);
		return std::invoke(obj, ImGuiSelectableFlags_None, size);
	};

	for (auto itr = data_begin; itr != data_end; ++itr)
	{
		auto obj = itr._Unwrapped( );

		const auto obj_selected = obj->selected( );
		const auto obj_pressed  = [&]
		{
			const auto ret = obj_invoke(*obj);
			if constexpr (Sameline)
			{
				if (itr != pre_end)
					ImGui::SameLine( );
			}
			return ret;
		}( );

		if (obj_selected)
		{
			if (!page_active)
				page_active = (obj);
			continue;
		}
		if (obj_pressed)
		{
			const auto itr_next = std::next(itr);

			if (!page_active)
			{
				const auto selected_later = std::ranges::find_if(itr_next, data_end, &widgets::selectable_base::selected);
				runtime_assert(selected_later != data_end);
				page_active = selected_later._Unwrapped( );
			}
			if (itr_next != data_end)
			{
				if constexpr (!Sameline)
				{
					std::ranges::for_each(itr_next, data_end, obj_invoke);
				}
				else
				{
					for (auto& p: std::span(itr_next, pre_end))
					{
						obj_invoke(p);
						ImGui::SameLine( );
					}

					obj_invoke(*pre_end);
				}
			}
			page_active->deselect( );
			obj->select( );
			return {(obj), true};
		}
	}
	return {(page_active), false};
}

void vertical_pages_renderer::render( )
{
	selected_info selected;

	if (!this->begin({ }, {pages_.size( ), static_cast<float>(longest_string__), size_info::WORD}, true))
		return this->end( );
	{
		const auto pop = push_style_var(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));
		(void)pop;

		selected = _Render_and_select<false>(pages_, [](const abstract_page&) { return ImVec2(0, 0); });
	}
	this->end( );

	ImGui::SameLine( );

	if (selected.activated)
		selected_group__.show( );
	selected_group__.begin( );
	{
		selected.ptr->render( );
	}
	selected_group__.end( );
}

void vertical_pages_renderer::init( )
{
	abstract_pages_renderer::init( );

	auto sizes       = _Sizes(pages_);
	longest_string__ = *std::ranges::max_element(sizes, std::less<size_t>( ));
}

void horizontal_pages_renderer::render( )
{
	selected_info selected;

	const auto char_size = _Get_char_size( ).x;

	size_info  size_x, size_y;
	const auto pages_per_line = /*std::min(per_line_limit__, pages_.size( ))*/pages_.size( );
#ifdef CHEAT_GUI_HORIZONTAL_PAGES_RENDERER_USE_LONGEST_STRING
	size_x = {pages_per_line, static_cast<float>(longest_string__), size_info::WORD};
	//size_y = {pages_.size( ) / pages_per_line, static_cast<float>(longest_string__) * pages_per_line, size_info::WORD};
#else
	const auto extra_size = char_size / (ImGui::GetStyle( ).ItemInnerSpacing.x * pages_.size( ));
	size_x = {1, chars_count__ + extra_size, size_info::WORD};
	//runtime_assert(per_line_limit__!=-1);
#endif

	if (!this->begin(size_x, size_y, true))
		return this->end( );
	{
		const auto pop = push_style_var(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));
		(void)pop;

		//ImGui::SameLine(0, indent_headers); //for first selectable
		selected = _Render_and_select<true>(pages_, [&](const pages_storage_data& p)
		{
			return ImVec2(
#ifdef CHEAT_GUI_HORIZONTAL_PAGES_RENDERER_USE_LONGEST_STRING
						  longest_string__
#else
						  _Get_num_chars(p)
#endif
						  * char_size, 0);
		});
	}
	this->end( );

	//ImGui::Indent(indent_page);
	if (selected.activated)
		selected_group__.show( );
	selected_group__.begin( );
	{
		selected.ptr->render( );
	}
	selected_group__.end( );
	//ImGui::Unindent(indent_page);
}

void horizontal_pages_renderer::init( )
{
	abstract_pages_renderer::init( );

	auto sizes = _Sizes(pages_);

#ifdef CHEAT_GUI_HORIZONTAL_PAGES_RENDERER_USE_LONGEST_STRING
	longest_string__ = *std::ranges::max_element(sizes, std::less<size_t>( ));
#else
	for (auto s: sizes)
		chars_count__ += s;
#endif
}
#endif

//------------------------------
struct shared_child_windows_storage::storage_type: std::vector<shared_window>
{
};

shared_child_windows_storage::shared_child_windows_storage( )
{
	storage_ = std::make_unique<storage_type>( );
}

shared_child_windows_storage::~shared_child_windows_storage( ) = default;

size_t shared_child_windows_storage::size( ) const
{
	return storage_->size( );
}

bool shared_child_windows_storage::empty( ) const
{
	return storage_->empty( );
}

void shared_child_windows_storage::add(const shared_window& window)
{
	(void)this;
	storage_->push_back(window);
}

const shared_child_windows_storage::shared_window& shared_child_windows_storage::operator[](size_t i) const
{
	return storage_->operator[](i);
}

const shared_child_windows_storage::shared_window* shared_child_windows_storage::begin( ) const
{
	return storage_->_Unchecked_begin( );
}

const shared_child_windows_storage::shared_window* shared_child_windows_storage::end( ) const
{
	return storage_->_Unchecked_end( );
}

non_abstract_label::non_abstract_label(tools::string_wrapper&& label)
	: label_(std::move(label))
{
}

non_abstract_label::~non_abstract_label( )
{
}

const tools::string_wrapper& non_abstract_label::label( ) const
{
	return label_;
}

static ImVec2 _Chars_to_size(size_t count)
{
	const auto& chr_size = _Get_char_size( );
	//const auto& style    = ImGui::GetStyle( );

	//const auto frame_padding = style.FramePadding.x * 2.f;

	return {chr_size.x * count, chr_size.y};
}

class tab_bar::item: widgets::selectable
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

//todo: made tab bar file cpp/h
enum class directions :uint8_t
{
	HORISONTAL,
	VERTICAL
};

enum class size_modes
{
	UNSET,
	STATIC,
	AUTO
};

struct tab_bar::impl
{
	widgets::child_frame_window wnd;
	std::vector<item>           items;

	size_modes size_mode = size_modes::UNSET;
	directions dir       = directions::HORISONTAL;

	std::optional<directions> temp_dir;
};

tab_bar::tab_bar( )
{
	impl_ = std::make_unique<impl>( );
}

tab_bar::~tab_bar( ) = default;

void tab_bar::add(const shared_label& label)
{
	(void)this;
	auto& items = impl_->items;

	items.push_back(label);
	if (items.size( ) == 1)
		items.front( ).select( );
}

size_t tab_bar::selected( ) const
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
void tab_bar::calc_size_static( )
{
	auto& mode = impl_->size_mode;
	if (mode == size_modes::STATIC)
		return;
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
	mode = size_modes::STATIC;
}

// ReSharper disable once CppMemberFunctionMayBeConst
void tab_bar::call_size_auto( )
{
	auto& mode = impl_->size_mode;
	if (mode == size_modes::AUTO)
		return;
	std::ranges::for_each(impl_->items, &item::set_size_auto);
	mode = size_modes::AUTO;
}

// ReSharper disable once CppMemberFunctionMayBeConst
void tab_bar::make_horisontal( )
{
	if (impl_->dir == directions::HORISONTAL)
		return;

	impl_->temp_dir = directions::HORISONTAL;
}

// ReSharper disable once CppMemberFunctionMayBeConst
void tab_bar::make_vertical( )
{
	if (impl_->dir == directions::VERTICAL)
		return;

	impl_->temp_dir = directions::VERTICAL;
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
	if (impl_->temp_dir.has_value( ))
	{
		impl_->dir = *impl_->temp_dir;
		impl_->temp_dir.reset( );
	}

	// ReSharper disable once CppTooWideScopeInitStatement
	const auto wnd_size = [&]
	{
		const auto& style = ImGui::GetStyle( );

		//const auto indent_headers = style.ItemInnerSpacing.x;
		//const auto indent_page = style./*ItemInnerSpacing*/ItemSpacing.x; //ImGui use ItemSpacing by default

		const auto frame_padding = style.FramePadding * 2.f;

		const auto transform_helper = [&]<typename T>(T&& xy)
		{
			return impl_->items
				   | std::views::transform(&item::size)
				   | std::views::transform(/*&ImVec2::x*/xy);
		};

		ImVec2 size;
		if (impl_->dir == directions::HORISONTAL)
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

			size.x = frame_padding.x +                                 //space before and after
					 /*indent_headers +*/                              //to indent first selectable
					 reserve_x( ) +                                    //reserve width for all strings / whole data
					 style.ItemSpacing.x * (impl_->items.size( ) - 1); //space between all headers

			size.y = frame_padding.y + //space before and after
					 reserve_y( );     //word height
		}
		else if (impl_->dir == directions::VERTICAL)
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

			size.y = frame_padding.y +                                 //space before and after
					 reserve_y( ) +                                    //all strings height						                            
					 style.ItemSpacing.y * (impl_->items.size( ) - 1); //space between all string
		}
		else
			runtime_assert("Unknown direction");

		return size;
	}( );

	if (impl_->wnd.begin(wnd_size))
	{
		const auto temp_center = push_style_var(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));

		const auto want_sameline = impl_->dir == directions::HORISONTAL;
		for (size_t i = 0; i < impl_->items.size( ); i++)
		{
			auto& item = impl_->items[i];
			if (item.render( ))
			{
				impl_->items[this->selected( )].deselect( );
				item.select( );
			}
			if (want_sameline && i < impl_->items.size( ) - 1)
				ImGui::SameLine( );
		}
	}
	impl_->wnd.end( );
}
