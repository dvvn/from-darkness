#pragma once
// ReSharper disable once CppUnusedIncludeDirective
#include <compare>

namespace dhooks::detail
{
	struct target_fn
	{
		void* target; // [In] Address of the target function.

		target_fn(void* target = nullptr)
			: target(target)
		{
		}

		bool operator==(const target_fn& other) const = default;
	};
}
