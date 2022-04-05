module;

#include <functional>
#include <future>

export module cheat.hooks:loader;

export namespace cheat::hooks
{
	struct hook_data
	{
		using updater = bool(*)();
		updater start, stop;
	};

	void add(hook_data data);

	std::future<bool> start( );
	void stop();
}