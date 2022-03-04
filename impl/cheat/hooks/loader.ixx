module;

#include <functional>
#include <future>

export module cheat.hooks.loader;
import dhooks;

using dhooks::hook_holder_data;

using stored_hook = std::unique_ptr<hook_holder_data>;
using hook_creator = std::function<stored_hook( )>;

void register_hook(hook_creator&& creator);

export namespace cheat::hooks::loader
{
	template<typename T>
	void add( )
	{
		register_hook([]( )->stored_hook
		{
			return std::make_unique<T>( );
		});
	}

	bool start( );
	std::future<bool> start_async( );
	void stop(bool force = false);

	/*class loader :public console::lifetime_notification<loader>, public nstd::one_instance<loader>
	{
	public:


	};*/
}