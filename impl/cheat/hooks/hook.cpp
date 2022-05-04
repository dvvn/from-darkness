module;

#include <nstd/runtime_assert_core.h>
#include <nstd/format.h>

#include <string>

module cheat.hooks.hook;
import cheat.console;

using namespace cheat::hooks;

hook::~hook( )
{
	//purecall here
	//hook::disable( );
}

void hook::init(entry_type&& entry)
{
	using std::swap;
	swap(entry_, entry);
}

template<typename M>
static void _Log(const hook* h, const M msg) noexcept
{
	cheat::console::log("{}: {}", [h]( ) noexcept
	{
		return h->name( );
	}, std::move(msg));
}

bool hook::enable( ) runtime_assert_noexcept
{
	if(!entry_.create( ) && !entry_.created( ))
	{
		_Log(this, "created error!");
		return false;
	}
	if(!entry_.enable( ))
	{
		_Log(this, [this]( ) noexcept -> std::string_view
		{
			return entry_.enabled( ) ? "already hooked":"enable error!";
		});
		return false;
	}
	_Log(this, "hooked");
	return true;
}

bool hook::disable( ) runtime_assert_noexcept
{
	const auto ok = entry_.disable( );
	_Log(this, [ok, this]( ) noexcept -> std::string_view
	{
		if(ok)
			return "unhooked";
		if(!entry_.enabled( ))
			return "already unhooked";
		if(entry_.created( ))
			return "unhook error!";
		return "not created!";
	});
	return ok;
}

void* hook::get_original_method( ) const runtime_assert_noexcept
{
	return entry_.get_original_method( );
}

std::string hook::name( ) const noexcept
{
	std::string ret;
	if(this->is_static( ))
	{
		const auto fn_name = dynamic_cast<const static_base*>(this)->function_name( );
		ret = {fn_name.begin( ),fn_name.end( )};
	}
	else
	{
		const auto base = dynamic_cast<const class_base*>(this);
		const auto class_name = base->class_name( );
		const auto fn_name = base->function_name( );
		ret.reserve(class_name.size( ) + 2 + fn_name.size( ));
		ret += class_name;
		ret += ':';
		ret += ':';
		ret += fn_name;
	}
	return ret;
}

