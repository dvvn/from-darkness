#pragma once

#include "hde.h"

#if defined(DHOOKS_X86)
#include "hde32.h"
#elif defined(DHOOKS_X64)
#include "hde64.h"
#endif

namespace dhooks::detail
{
#if defined(DHOOKS_X86)

	constexpr auto _HDE_disasm = hde::hde32_disasm;
	using HDE_data = hde::hde32s;
	//constexpr auto TRAMPOLINE_MAX_SIZE = MEMORY_SLOT_SIZE;
#elif defined(DHOOKS_X64)
     constexpr auto _HDE_disasm= hde64_disasm;
    using HDE_data=hde64s;
    //constexpr auto TRAMPOLINE_MAX_SIZE = MEMORY_SLOT_SIZE - sizeof(JMP_ABS);
#endif
}
