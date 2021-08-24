#include "trampoline.h"

#include "hde/include.h"

#include "nstd/memory block.h"

using namespace dhooks::detail;
using namespace dhooks;
//bool dhooks::detail::_Is_code_padding(LPBYTE pInst, UINT size)
//{
//	
//}
//
//bool dhooks::detail::_Is_address_readable(LPVOID addr, size_t mem_size)
//{
//	return memory_block(addr, mem_size).readable_ex( );
//}
//
//bool dhooks::detail::_Is_address_executable(LPVOID addr, size_t mem_size)
//{
//	return memory_block(addr, mem_size).executable( );
//}

trampoline::trampoline( )
{
	trampoline__ = std::make_unique<char[]>(this->buffer_size( ));
}

uint8_t* trampoline::buffer( )
{
	(void)this;
	return reinterpret_cast<uint8_t*>(trampoline__.get( ));
}

size_t trampoline::buffer_size( ) const
{
	(void)this;
	//return sizeof(uintptr_t) * trampoline_.capacity( );
	return sizeof(uintptr_t) * 8;
}

bool trampoline::fix_page_protection( )
{
	runtime_assert(!old_protection__.has_value());

	const auto buff      = this->buffer( );
	const auto buff_size = this->buffer_size( );

	if (!nstd::memory_block(buff, buff_size).executable( ))
	{
		try
		{
			old_protection__ = {buff, buff_size, PAGE_EXECUTE_READWRITE};
		}
		catch (const std::exception&)
		{
			return false;
		}
	}

	return true;
}

bool trampoline::create( )
{
	auto& ct = *this;

#ifdef DHOOKS_X64
    CALL_ABS call = {
        0xFF, 0x15, 0x00000002, // FF15 00000002: CALL [RIP+8]
        0xEB, 0x08,             // EB 08:         JMP +10
        0x0000000000000000ULL   // Absolute destination address
    };
    JMP_ABS jmp = {
        0xFF, 0x25, 0x00000000, // FF25 00000000: JMP [RIP+6]
        0x0000000000000000ULL   // Absolute destination address
    };
    JCC_ABS jcc = {
        0x70, 0x0E,             // 7* 0E:         J** +16
        0xFF, 0x25, 0x00000000, // FF25 00000000: JMP [RIP+6]
        0x0000000000000000ULL   // Absolute destination address
    };
#else
	CALL_REL call = {
		0xE8,      // E8 xxxxxxxx: CALL +5+xxxxxxxx
		0x00000000 // Relative destination address
	};
	JMP_REL jmp = {
		0xE9,      // E9 xxxxxxxx: JMP +5+xxxxxxxx
		0x00000000 // Relative destination address
	};
	JCC_REL jcc = {
		0x0F, 0x80, // 0F8* xxxxxxxx: J** +6+xxxxxxxx
		0x00000000  // Relative destination address
	};
#endif

	UINT8     old_pos  = 0;
	UINT8     new_pos  = 0;
	ULONG_PTR jmp_dest = 0;     // Destination address of an internal jump.
	bool      finished = false; // Is the function completed?
#ifdef DHOOKS_X64
    UINT8 instBuf[16];
#endif

	ct.patch_above = false;
	ct.ips_count   = 0;

	do
	{
		HDE_data hs;

		const auto old_inst = reinterpret_cast<ULONG_PTR>(ct.target) + old_pos;
		const auto new_inst = reinterpret_cast<ULONG_PTR>(ct.trampoline__.get( )) + new_pos;

		auto copy_size = _HDE_disasm(reinterpret_cast<LPVOID>(old_inst), &hs);
		if (hs.flags & hde::F_ERROR)
			return false;

		auto copy_src = reinterpret_cast<LPVOID>(old_inst);
		if (old_pos >= sizeof(JMP_REL))
		{
			// The trampoline function is long enough.
			// Complete the function with the jump to the target function.
#ifdef DHOOKS_X64
            jmp.address = old_inst;
#else
			jmp.operand = static_cast<UINT32>(old_inst - (new_inst + sizeof(decltype(jmp))));
#endif
			copy_src  = &jmp;
			copy_size = sizeof(decltype(jmp));

			finished = true;
		}
#ifdef DHOOKS_X64
        else if ((hs.modrm & 0xC7) == 0x05)
        {
            // Instructions using RIP relative addressing. (ModR/M = 00???101B)

            // Modify the RIP relative address.
            PUINT32 pRelAddr;

            // Avoid using memcpy to reduce the footprint.
            std::memcpy(instBuf, (LPBYTE)old_inst, copy_size);

            copy_src = instBuf;

            // Relative address is stored at (instruction length - immediate value length - 4).
            pRelAddr  = (PUINT32)(instBuf + hs.len - ((hs.flags & 0x3C) >> 2) - 4);
            *pRelAddr = static_cast<UINT32>(old_inst + hs.len + static_cast<INT32>(hs.disp.disp32) - (new_inst + hs.len));

            // Complete the function if JMP (FF /4).
            if (hs.opcode == 0xFF && hs.modrm_reg == 4)
                finished = true;
        }
#endif
		else if (hs.opcode == 0xE8)
		{
			// Direct relative CALL
			const auto dest = old_inst + hs.len + static_cast<INT32>(hs.imm.imm32);
#ifdef DHOOKS_X64
            call.address = dest;
#else
			call.operand = static_cast<UINT32>(dest - (new_inst + sizeof(call)));
#endif
			copy_src  = &call;
			copy_size = sizeof(decltype(call));
		}
		else if ((hs.opcode & 0xFD) == 0xE9)
		{
			// Direct relative JMP (EB or E9)
			auto dest = old_inst + hs.len;

			if (hs.opcode == 0xEB) // isShort jmp
				dest += static_cast<INT8>(hs.imm.imm8);
			else
				dest += static_cast<INT32>(hs.imm.imm32);

			// Simply copy an internal jump.
			if (reinterpret_cast<ULONG_PTR>(ct.target) <= dest && dest < reinterpret_cast<ULONG_PTR>(ct.target) + sizeof(JMP_REL))
			{
				if (jmp_dest < dest)
					jmp_dest = dest;
			}
			else
			{
#ifdef DHOOKS_X64
                jmp.address = dest;
#else
				jmp.operand = static_cast<UINT32>(dest - (new_inst + sizeof(decltype(jmp))));
#endif
				copy_src  = &jmp;
				copy_size = sizeof(decltype(jmp));

				// Exit the function If it is not in the branch
				finished = old_inst >= jmp_dest;
			}
		}
		else if ((hs.opcode & 0xF0) == 0x70 || (hs.opcode & 0xFC) == 0xE0 || (hs.opcode2 & 0xF0) == 0x80)
		{
			// Direct relative Jcc
			auto dest = old_inst + hs.len;

			if ( // Jcc
				(hs.opcode & 0xF0) == 0x70 ||
				// LOOPNZ/LOOPZ/LOOP/JECXZ
				(hs.opcode & 0xFC) == 0xE0)
				dest += static_cast<INT8>(hs.imm.imm8);
			else
				dest += static_cast<INT32>(hs.imm.imm32);

			// Simply copy an internal jump.
			if (reinterpret_cast<ULONG_PTR>(ct.target) <= dest && dest < reinterpret_cast<ULONG_PTR>(ct.target) + sizeof(JMP_REL))
			{
				if (jmp_dest < dest)
					jmp_dest = dest;
			}
			else if ((hs.opcode & 0xFC) == 0xE0)
			{
				// LOOPNZ/LOOPZ/LOOP/JCXZ/JECXZ to the outside are not supported.
				return false;
			}
			else
			{
				const UINT8 cond = (hs.opcode != 0x0F ? hs.opcode : hs.opcode2) & 0x0F;
#ifdef DHOOKS_X64
                // Invert the condition in x64 mode to simplify the conditional jump logic.
                jcc.opcode  = 0x71 ^ cond;
                jcc.address = dest;
#else
				jcc.opcode1 = 0x80 | cond;
				jcc.operand = static_cast<UINT32>(dest - (new_inst + sizeof(decltype(jcc))));
#endif
				copy_src  = &jcc;
				copy_size = sizeof jcc;
			}
		}
		else if ((hs.opcode & 0xFE) == 0xC2)
		{
			// RET (C2 or C3)

			// Complete the function if not in a branch.
			finished = old_inst >= jmp_dest;
		}

		// Can't alter the instruction length in a branch.
		if (old_inst < jmp_dest && copy_size != hs.len)
			return false;

		// Trampoline function is too large.
		if (new_pos + copy_size > this->buffer_size( ))
			return false;

		// Trampoline function has too many instructions.
		if (ct.ips_count >= ct.old_ips.size( ))
			return false;

		ct.old_ips[ct.ips_count] = old_pos;
		ct.new_ips[ct.ips_count] = new_pos;
		ct.ips_count++;

		std::memcpy((ct.buffer( )) + new_pos, copy_src, copy_size);

		new_pos += copy_size;
		old_pos += hs.len;
	}
	while (!finished);

	using namespace nstd;

	// Is there enough place for a long jump?
	if (old_pos < sizeof(JMP_REL) && !memory_block(address(ct.target) + old_pos, sizeof(JMP_REL) - old_pos).code_padding( ))
	{
		// Is there enough place for a short jump?
		if (old_pos < sizeof(JMP_REL_SHORT) && !memory_block(address(ct.target) + old_pos, sizeof(JMP_REL_SHORT) - old_pos).code_padding( ))
			return false;

		// Can we place the long jump above the function?
		if (!memory_block(address(ct.target) - sizeof(JMP_REL)).executable( ))
			return false;
		if (!memory_block(address(ct.target) - sizeof(JMP_REL), sizeof(JMP_REL)).code_padding( ))
			return false;

		ct.patch_above = true;
	}

#ifdef DHOOKS_X64
    // Create a relay function.
    jmp.address = reinterpret_cast<ULONG_PTR>(ct.pDetour);

    ct.pRelay = static_cast<LPBYTE>(ct.pTrampoline.get( )) + new_pos;
    /*utl_*/
    memcpy(ct.pRelay, &jmp, sizeof jmp);
#endif

	return true;
}
