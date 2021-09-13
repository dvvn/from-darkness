// ReSharper disable CppMemberFunctionMayBeConst
#include "selectable base.h"

using namespace cheat;
using namespace gui::widgets;
using namespace gui::tools;

struct selectable_base2::impl
{
	two_way_callback on_select;
	bool             selected = false;
};

selectable_base2::selectable_base2( )
{
	impl_ = std::make_unique<impl>( );
}

selectable_base2::~selectable_base2( )                                     = default;
selectable_base2::selectable_base2(selectable_base2&&) noexcept            = default;
selectable_base2& selectable_base2::operator=(selectable_base2&&) noexcept = default;

void selectable_base2::select(const callback_data& data)
{
	impl_->selected = true;
	impl_->on_select(true, data);
}

void selectable_base2::deselect(const callback_data& data)
{
	impl_->selected = false;
	impl_->on_select(false, data);
}

void selectable_base2::toggle(const callback_data& data)
{
	auto& selected = impl_->selected;
	selected       = !selected;
	impl_->on_select(selected, data);
}

bool selectable_base2::selected( ) const
{
	return impl_->selected;
}

void selectable_base2::add_selected_callback(callback_info&& info, two_way_callback::ways way)
{
	impl_->on_select.add(std::move(info), way);
}
