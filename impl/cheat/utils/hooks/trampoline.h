#pragma once

namespace cheat::utl::hooks::detail
{
#pragma pack(push, 1)
    // ReSharper disable CppInconsistentNaming

    // Structs for writing x86/x64 instructions.

    // 8-bit relative jump.
    struct JMP_REL_SHORT
    {
        UINT8 opcode; // EB xx: JMP +2+xx
        UINT8 operand;
    };

    // 32-bit direct relative jump/call.
    struct JMP_REL
    {
        UINT8 opcode;   // E9/E8 xxxxxxxx: JMP/CALL +5+xxxxxxxx
        UINT32 operand; // Relative destination address
    };
    using CALL_REL = JMP_REL;
    // 64-bit indirect absolute jump.

    struct JMP_ABS
    {
        UINT8 opcode0; // FF25 00000000: JMP [+6]
        UINT8 opcode1;
        UINT32 dummy;
        UINT64 address; // Absolute destination address
    };

    // 64-bit indirect absolute call.
    struct CALL_ABS
    {
        UINT8 opcode0; // FF15 00000002: CALL [+6]
        UINT8 opcode1;
        UINT32 dummy0;
        UINT8 dummy1; // EB 08:         JMP +10
        UINT8 dummy2;
        UINT64 address; // Absolute destination address
    };

    // 32-bit direct relative conditional jumps.
    struct JCC_REL
    {
        UINT8 opcode0; // 0F8* xxxxxxxx: J** +6+xxxxxxxx
        UINT8 opcode1;
        UINT32 operand; // Relative destination address
    };

    // 64bit indirect absolute conditional jumps that x64 lacks.
    struct JCC_ABS
    {
        UINT8 opcode; // 7* 0E:         J** +16
        UINT8 dummy0;
        UINT8 dummy1; // FF25 00000000: JMP [+6]
        UINT8 dummy2;
        UINT32 dummy3;
        UINT64 address; // Absolute destination address
    };

    // ReSharper restore CppInconsistentNaming
#pragma pack(pop)

    class trampoline_target
    {
    public:
        LPVOID target; // [In] Address of the target function.

        trampoline_target(LPVOID target = nullptr): target(target)
        {
        }

        auto operator<=>(const trampoline_target& other) const = default;
    };

    class trampoline: public trampoline_target
    {
    BOOST_MOVABLE_BUT_NOT_COPYABLE(trampoline);
    public:
        trampoline( );
        virtual ~trampoline( );

        trampoline(trampoline&&) = default;
        trampoline& operator=(trampoline&&)= default;

        using ips_type = array<UINT8, 8>;

        LPVOID detour = nullptr; // [In] Address of the detour function.

#ifdef UTILS_X64
		LPVOID pRelay=nullptr; // [Out] Address of the relay function.
#endif
        bool patch_above = false; // [Out] Should use the hot patch area?
        UINT ips_count = 0;       // [Out] Number of the instruction boundaries.

        ips_type old_ips; // [Out] Instruction boundaries of the target function.
        ips_type new_ips; // [Out] Instruction boundaries of the trampoline function.

        uint8_t* buffer( );
        size_t   buffer_size( ) const;

        bool fix_page_protection( );
        bool create( );

    private:
        optional<DWORD> old_protection__;
        //boost::container::small_vector<uintptr_t, 8> trampoline_; // [In] Buffer address for the trampoline and relay function.
        unique_ptr<char[]> trampoline__;
    };

    bool _Is_code_padding(LPBYTE pInst, UINT size);
    bool _Is_address_readable(LPVOID addr, size_t mem_size = sizeof(LPVOID));
    bool _Is_address_executable(LPVOID addr, size_t mem_size = sizeof(LPVOID));
}
