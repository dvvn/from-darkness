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

selectable_base::~selectable_base( )                                    = default;
selectable_base::selectable_base(selectable_base&&) noexcept            = default;
selectable_base& selectable_base::operator=(selectable_base&&) noexcept = default;

void selectable_base::select( )
{
	impl_->selected = true;
	impl_->on_select(true);
}

void selectable_base::deselect( )
{
	impl_->selected = false;
	impl_->on_select(false);
}

void selectable_base::toggle( )
{
	auto& selected = impl_->selected;
	selected       = !selected;
	impl_->on_select(selected);
}

bool selectable_base::selected( ) const
{
	return impl_->selected;
}

void selectable_base::add_selected_callback(callback_info&& info, two_way_callback::ways way)
{
	impl_->on_select[way].add(std::move(info));
}

#ifdef CHEAT_GUI_CALLBACK_HAVE_INDEX
bool selectable_base::erase_selected_callback(const callback_id& ids, two_way_callback::ways way)
{
	return impl_->on_select[way].erase(ids);
}
#endif
