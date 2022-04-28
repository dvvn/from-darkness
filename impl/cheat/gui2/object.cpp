module;

#include <vector>

module cheat.gui2.object;

using namespace cheat::gui2;

auto objects_storage::begin( ) noexcept -> iterator
{
	return storage_.begin( );
}

auto objects_storage::begin( ) const noexcept -> const_iterator
{
	return storage_.begin( );
}

auto objects_storage::end( ) noexcept  -> iterator
{
	return storage_.end( );
}

auto objects_storage::end( ) const noexcept -> const_iterator
{
	return storage_.end( );
}

size_t objects_storage::size( ) const noexcept
{
	return storage_.size( );
}

bool objects_storage::empty( ) const noexcept
{
	return storage_.empty( );
}

auto objects_storage::add(value_type&& value) noexcept -> pointer
{
	const auto ptr = value.get( );
	storage_.push_back(std::move(value));
	return ptr;
}

//--------------

objects_storage* const child_storage::child( ) noexcept
{
	return std::addressof(storage_);
}

const objects_storage* const child_storage::child( ) const noexcept
{
	return std::addressof(storage_);
}

//--------------

bool object::handle_event(event* const ev) noexcept
{
	for(auto& child_obj : *this->child( ))
	{
		if(!child_obj->handle_event(ev))
			return false;
	}

	return true;
}