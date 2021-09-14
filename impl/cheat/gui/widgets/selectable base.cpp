// ReSharper disable CppMemberFunctionMayBeConst
#include "selectable base.h"

using namespace cheat;
using namespace gui::widgets;
using namespace gui::tools;

struct selectable_base::impl
{
	two_way_callback on_select;
	bool             selected = false;
};

selectable_base::selectable_base( )
{
	impl_ = std::make_unique<impl>( );
}

selectable_base::~selectable_base( )                                     = default;
selectable_base::selectable_base(selectable_base&&) noexcept            = default;
selectable_base& selectable_base::operator=(selectable_base&&) noexcept = default;

void selectable_base::select(const callback_data& data)
{
	impl_->selected = true;
	impl_->on_select(true, data);
}

void selectable_base::deselect(const callback_data& data)
{
	impl_->selected = false;
	impl_->on_select(false, data);
}

void selectable_base::toggle(const callback_data& data)
{
	auto& selected = impl_->selected;
	selected       = !selected;
	impl_->on_select(selected, data);
}

bool selectable_base::selected( ) const
{
	return impl_->selected;
}

void selectable_base::add_selected_callback(callback_info&& info, two_way_callback::ways way)
{
	impl_->on_select.add(std::move(info), way);
}
