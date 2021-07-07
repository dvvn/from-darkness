#pragma once

#if defined(_M_IX86) || defined(__i386__)
#include "hde32.h"
#else
#include "hde64.h"
#endif

namespace cheat::utl::hooks::detail_hde
{
#if defined(_M_IX86) || defined(__i386__)

	constexpr auto _HDE_disasm = hde32_disasm;
	using HDE_data = hde32s;
	//constexpr auto TRAMPOLINE_MAX_SIZE = MEMORY_SLOT_SIZE;
#else
     constexpr auto _HDE_disasm= hde64_disasm;
    using HDE_data=hde64s;
    constexpr auto TRAMPOLINE_MAX_SIZE = MEMORY_SLOT_SIZE - sizeof(JMP_ABS);
#endif
}
