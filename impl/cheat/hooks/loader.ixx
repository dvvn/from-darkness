module;

#include <future>
#include <compare>

export module cheat.hooks:loader;

export namespace cheat::hooks
{
	struct hook_data
	{
		using updater = bool(*)();
		updater start, stop;
	};

	void add(hook_data data) noexcept;

	std::future<bool> start( ) noexcept;
	void stop( ) noexcept;
}