module;

#include <nstd/runtime_assert_core.h>

export module cheat.hooks.initializer;

export namespace cheat::hooks
{
	void init_basic( ) runtime_assert_noexcept;
	void init_all( ) runtime_assert_noexcept;
}