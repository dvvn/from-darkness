module;

#include <functional>
#include <string>

module fds.hook;
import fds.logger.system_console;

hook::~hook()
{
    // purecall here
    // hook::disable( );
}

hook::hook()
{
    entry_ = dhooks::create_hook_entry();
}

template <typename M>
static void _Log(const hook* h, const M& msg)
{
    fds::logger_system_console->log("{}: {}", std::bind_front(&hook::name, h), msg);
}

bool hook::enable()
{
    if (!entry_->created() && !entry_->create())
    {
        _Log(this, "created error!");
        return false;
    }
    if (!entry_->enable())
    {
        _Log(this, [this] {
            return entry_->enabled() ? "already hooked" : "enable error!";
        });
        return false;
    }
    _Log(this, "hooked");
    return true;
}

bool hook::disable()
{
    const auto ok = entry_->disable();
    _Log(this, [ok, this] {
        if (ok)
            return "unhooked";
        if (!entry_->enabled())
            return "already unhooked";
        if (entry_->created())
            return "unhook error!";
        return "not created!";
    });
    return ok;
}

bool hook::initialized() const
{
    return entry_->get_target_method() && entry_->get_replace_method();
}

bool hook::active() const
{
    return entry_->enabled();
}

void* hook::get_original_method() const
{
    return entry_->get_original_method();
}

void hook::init(const function_getter target, const function_getter replace)
{
    entry_->set_target_method(target);
    entry_->set_replace_method(replace);
}

#if 0
std::string hook::name( ) const
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
#endif
