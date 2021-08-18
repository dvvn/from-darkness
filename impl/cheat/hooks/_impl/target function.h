#pragma once

namespace cheat::hooks::detail
{
	struct target_fn
	{
		LPVOID target; // [In] Address of the target function.

		target_fn(const LPVOID target = nullptr): target(target)
		{
		}

		auto operator<=>(const target_fn& other) const = default;
	};
}
