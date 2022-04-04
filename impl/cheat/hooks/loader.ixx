module;

#include <functional>
#include <future>

export module cheat.hooks:loader;
import dhooks;

using stored_hook = std::unique_ptr<dhooks::hook_holder_data>;
using hook_creator = std::function<stored_hook( )>;

void register_hook(hook_creator&& creator);

export namespace cheat::hooks
{
	template<typename T>
	void add( )
	{
		register_hook([]
		{
			return std::make_unique<T>( );
		});
	}

	std::future<bool> start( );
	void stop(bool force = false);
}