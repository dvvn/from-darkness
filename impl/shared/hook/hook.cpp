#include "hook.h"
#include "debug/log.h"
#include "functional/ignore.h"

#ifdef _DEBUG
#include "string/char.h"
#endif

#include <algorithm>
#include <cassert>
#include <ranges>

#if __has_include(<subhook.h>)
#include <subhook.h>

#define _SUBHOOK_WRAP(_FN_)                                 \
    static auto subhook_##_FN_(void *ptr)                   \
    {                                                       \
        return subhook_##_FN_(static_cast<subhook_t>(ptr)); \
    }

#undef free

// ReSharper disable All
_SUBHOOK_WRAP(free);
_SUBHOOK_WRAP(install);
_SUBHOOK_WRAP(remove);
_SUBHOOK_WRAP(is_installed);
_SUBHOOK_WRAP(get_trampoline);
_SUBHOOK_WRAP(get_src);
_SUBHOOK_WRAP(get_dst);
// ReSharper restore All

#undef _SUBHOOK_WRAP

// ReSharper disable CppInconsistentNaming

enum
{
    C_NONE    = 0x00,
    C_MODRM   = 0x01,
    C_IMM8    = 0x02,
    C_IMM16   = 0x04,
    C_IMM_P66 = 0x10,
    C_REL8    = 0x20,
    C_REL32   = 0x40,
    C_GROUP   = 0x80,
    C_ERROR   = 0xff
};

enum
{
    PRE_ANY  = 0x00,
    PRE_NONE = 0x01,
    PRE_F2   = 0x02,
    PRE_F3   = 0x04,
    PRE_66   = 0x08,
    PRE_67   = 0x10,
    PRE_LOCK = 0x20,
    PRE_SEG  = 0x40,
    PRE_ALL  = 0xff
};

enum
{
    DELTA_OPCODES      = 0x4a,
    DELTA_FPU_REG      = 0xf1,
    DELTA_FPU_MODRM    = 0xf8,
    DELTA_PREFIXES     = 0x130,
    DELTA_OP_LOCK_OK   = 0x1a1,
    DELTA_OP2_LOCK_OK  = 0x1b9,
    DELTA_OP_ONLY_MEM  = 0x1cb,
    DELTA_OP2_ONLY_MEM = 0x1da
};

static constexpr uint8_t hde32_table[] = {
    0xa3, 0xa8, 0xa3, 0xa8, 0xa3, 0xa8, 0xa3, 0xa8, 0xa3, 0xa8, 0xa3, 0xa8, 0xa3, 0xa8, 0xa3, 0xa8, 0xaa, 0xaa, 0xaa,
    0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xac, 0xaa, 0xb2, 0xaa, 0x9f, 0x9f, 0x9f, 0x9f, 0xb5, 0xa3, 0xa3, 0xa4, 0xaa, 0xaa,
    0xba, 0xaa, 0x96, 0xaa, 0xa8, 0xaa, 0xc3, 0xc3, 0x96, 0x96, 0xb7, 0xae, 0xd6, 0xbd, 0xa3, 0xc5, 0xa3, 0xa3, 0x9f,
    0xc3, 0x9c, 0xaa, 0xaa, 0xac, 0xaa, 0xbf, 0x03, 0x7f, 0x11, 0x7f, 0x01, 0x7f, 0x01, 0x3f, 0x01, 0x01, 0x90, 0x82,
    0x7d, 0x97, 0x59, 0x59, 0x59, 0x59, 0x59, 0x7f, 0x59, 0x59, 0x60, 0x7d, 0x7f, 0x7f, 0x59, 0x59, 0x59, 0x59, 0x59,
    0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x9a, 0x88, 0x7d, 0x59, 0x50, 0x50, 0x50, 0x50, 0x59, 0x59, 0x59, 0x59,
    0x61, 0x94, 0x61, 0x9e, 0x59, 0x59, 0x85, 0x59, 0x92, 0xa3, 0x60, 0x60, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59,
    0x59, 0x59, 0x59, 0x59, 0x9f, 0x01, 0x03, 0x01, 0x04, 0x03, 0xd5, 0x03, 0xcc, 0x01, 0xbc, 0x03, 0xf0, 0x10, 0x10,
    0x10, 0x10, 0x50, 0x50, 0x50, 0x50, 0x14, 0x20, 0x20, 0x20, 0x20, 0x01, 0x01, 0x01, 0x01, 0xc4, 0x02, 0x10, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x01, 0xc0, 0xc2, 0x10, 0x11, 0x02, 0x03, 0x11, 0x03, 0x03, 0x04, 0x00, 0x00, 0x14, 0x00,
    0x02, 0x00, 0x00, 0xc6, 0xc8, 0x02, 0x02, 0x02, 0x02, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff,
    0xca, 0x01, 0x01, 0x01, 0x00, 0x06, 0x00, 0x04, 0x00, 0xc0, 0xc2, 0x01, 0x01, 0x03, 0x01, 0xff, 0xff, 0x01, 0x00,
    0x03, 0xc4, 0xc4, 0xc6, 0x03, 0x01, 0x01, 0x01, 0xff, 0x03, 0x03, 0x03, 0xc8, 0x40, 0x00, 0x0a, 0x00, 0x04, 0x00,
    0x00, 0x00, 0x00, 0x7f, 0x00, 0x33, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xbf, 0xff, 0xff, 0x00, 0x00,
    0x00, 0x00, 0x07, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xbf, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0xff,
    0x4a, 0x4a, 0x4a, 0x4a, 0x4b, 0x52, 0x4a, 0x4a, 0x4a, 0x4a, 0x4f, 0x4c, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a,
    0x4a, 0x55, 0x45, 0x40, 0x4a, 0x4a, 0x4a, 0x45, 0x59, 0x4d, 0x46, 0x4a, 0x5d, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a,
    0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x61, 0x63, 0x67, 0x4e, 0x4a, 0x4a, 0x6b, 0x6d, 0x4a, 0x4a, 0x45,
    0x6d, 0x4a, 0x4a, 0x44, 0x45, 0x4a, 0x4a, 0x00, 0x00, 0x00, 0x02, 0x0d, 0x06, 0x06, 0x06, 0x06, 0x0e, 0x00, 0x00,
    0x00, 0x00, 0x06, 0x06, 0x06, 0x00, 0x06, 0x06, 0x02, 0x06, 0x00, 0x0a, 0x0a, 0x07, 0x07, 0x06, 0x02, 0x05, 0x05,
    0x02, 0x02, 0x00, 0x00, 0x04, 0x04, 0x04, 0x04, 0x00, 0x00, 0x00, 0x0e, 0x05, 0x06, 0x06, 0x06, 0x01, 0x06, 0x00,
    0x00, 0x08, 0x00, 0x10, 0x00, 0x18, 0x00, 0x20, 0x00, 0x28, 0x00, 0x30, 0x00, 0x80, 0x01, 0x82, 0x01, 0x86, 0x00,
    0xf6, 0xcf, 0xfe, 0x3f, 0xab, 0x00, 0xb0, 0x00, 0xb1, 0x00, 0xb3, 0x00, 0xba, 0xf8, 0xbb, 0x00, 0xc0, 0x00, 0xc1,
    0x00, 0xc7, 0xbf, 0x62, 0xff, 0x00, 0x8d, 0xff, 0x00, 0xc4, 0xff, 0x00, 0xc5, 0xff, 0x00, 0xff, 0xff, 0xeb, 0x01,
    0xff, 0x0e, 0x12, 0x08, 0x00, 0x13, 0x09, 0x00, 0x16, 0x08, 0x00, 0x17, 0x09, 0x00, 0x2b, 0x09, 0x00, 0xae, 0xff,
    0x07, 0xb2, 0xff, 0x00, 0xb4, 0xff, 0x00, 0xb5, 0xff, 0x00, 0xc3, 0x01, 0x00, 0xc7, 0xff, 0xbf, 0xe7, 0x08, 0x00,
    0xf0, 0x02, 0x00};

enum
{
    F_MODRM         = 0x00000001,
    F_SIB           = 0x00000002,
    F_IMM8          = 0x00000004,
    F_IMM16         = 0x00000008,
    F_IMM32         = 0x00000010,
    F_DISP8         = 0x00000020,
    F_DISP16        = 0x00000040,
    F_DISP32        = 0x00000080,
    F_RELATIVE      = 0x00000100,
    F_2IMM16        = 0x00000800,
    F_ERROR         = 0x00001000,
    F_ERROR_OPCODE  = 0x00002000,
    F_ERROR_LENGTH  = 0x00004000,
    F_ERROR_LOCK    = 0x00008000,
    F_ERROR_OPERAND = 0x00010000,
    F_PREFIX_REPNZ  = 0x01000000,
    F_PREFIX_REPX   = 0x02000000,
    F_PREFIX_REP    = 0x03000000,
    F_PREFIX_66     = 0x04000000,
    F_PREFIX_67     = 0x08000000,
    F_PREFIX_LOCK   = 0x10000000,
    F_PREFIX_SEG    = 0x20000000,
    F_PREFIX_ANY    = 0x3f000000
};

enum
{
    PREFIX_SEGMENT_CS   = 0x2e,
    PREFIX_SEGMENT_SS   = 0x36,
    PREFIX_SEGMENT_DS   = 0x3e,
    PREFIX_SEGMENT_ES   = 0x26,
    PREFIX_SEGMENT_FS   = 0x64,
    PREFIX_SEGMENT_GS   = 0x65,
    PREFIX_LOCK         = 0xf0,
    PREFIX_REPNZ        = 0xf2,
    PREFIX_REPX         = 0xf3,
    PREFIX_OPERAND_SIZE = 0x66,
    PREFIX_ADDRESS_SIZE = 0x67
};

#pragma pack(push, 1)

using hde32s = struct
{
    uint8_t len;
    uint8_t p_rep;
    uint8_t p_lock;
    uint8_t p_seg;
    uint8_t p_66;
    uint8_t p_67;
    uint8_t opcode;
    uint8_t opcode2;
    uint8_t modrm;
    uint8_t modrm_mod;
    uint8_t modrm_reg;
    uint8_t modrm_rm;
    uint8_t sib;
    uint8_t sib_scale;
    uint8_t sib_index;
    uint8_t sib_base;

    union
    {
        uint8_t imm8;
        uint16_t imm16;
        uint32_t imm32;
    } imm;

    union
    {
        uint8_t disp8;
        uint16_t disp16;
        uint32_t disp32;
    } disp;

    uint32_t flags;
};

#pragma pack(pop)

// ReSharper restore CppInconsistentNaming

// ReSharper disable CppClangTidyBugproneAssignmentInIfCondition
// ReSharper disable CppClangTidyCppcoreguidelinesAvoidGoto
// ReSharper disable CppClangTidyClangDiagnosticImplicitIntConversion
// ReSharper disable CppLocalVariableMightNotBeInitialized
static unsigned int hde32_disasm(void *code, hde32s *hs)
{
    uint8_t x;
    uint8_t c;
    auto *p      = static_cast<uint8_t *>(code);
    uint8_t pref = 0;
    auto ht      = hde32_table;
    uint8_t m_mod;
    uint8_t m_reg;
    uint8_t m_rm;
    uint8_t disp_size = 0;

    memset(hs, 0, sizeof(hde32s));

    for (x = 16; x; x--)
        switch (c = *p++)
        {
        case 0xf3:
            hs->p_rep = c;
            pref |= PRE_F3;
            break;
        case 0xf2:
            hs->p_rep = c;
            pref |= PRE_F2;
            break;
        case 0xf0:
            hs->p_lock = c;
            pref |= PRE_LOCK;
            break;
        case 0x26:
        case 0x2e:
        case 0x36:
        case 0x3e:
        case 0x64:
        case 0x65:
            hs->p_seg = c;
            pref |= PRE_SEG;
            break;
        case 0x66:
            hs->p_66 = c;
            pref |= PRE_66;
            break;
        case 0x67:
            hs->p_67 = c;
            pref |= PRE_67;
            break;
        default:
            goto pref_done;
        }
pref_done:

    hs->flags = static_cast<uint32_t>(pref) << 23;

    if (!pref)
        pref |= PRE_NONE;

    if ((hs->opcode = c) == 0x0f)
    {
        hs->opcode2 = c = *p++;
        ht += DELTA_OPCODES;
    }
    else if (c >= 0xa0 && c <= 0xa3)
    {
        if (pref & PRE_67)
            pref |= PRE_66;
        else
            pref &= ~PRE_66;
    }

    auto opcode = c;
    auto cflags = ht[ht[opcode / 4] + opcode % 4];

    if (cflags == C_ERROR)
    {
        hs->flags |= F_ERROR | F_ERROR_OPCODE;
        cflags = 0;
        if ((opcode & -3) == 0x24)
            cflags++;
    }

    x = 0;
    if (cflags & C_GROUP)
    {
        auto t = *reinterpret_cast<const uint16_t *>(ht + (cflags & 0x7f));
        cflags = static_cast<uint8_t>(t);
        x      = static_cast<uint8_t>(t >> 8);
    }

    if (hs->opcode2)
    {
        ht = hde32_table + DELTA_PREFIXES;
        if (ht[ht[opcode / 4] + opcode % 4] & pref)
            hs->flags |= F_ERROR | F_ERROR_OPCODE;
    }

    if (cflags & C_MODRM)
    {
        hs->flags |= F_MODRM;
        hs->modrm = c = *p++;
        hs->modrm_mod = m_mod = c >> 6;
        hs->modrm_rm = m_rm = c & 7;
        hs->modrm_reg = m_reg = (c & 0x3f) >> 3;

        if (x && x << m_reg & 0x80)
            hs->flags |= F_ERROR | F_ERROR_OPCODE;

        if (!hs->opcode2 && opcode >= 0xd9 && opcode <= 0xdf)
        {
            uint8_t t = opcode - 0xd9;
            if (m_mod == 3)
            {
                ht = hde32_table + DELTA_FPU_MODRM + t * 8;
                t  = ht[m_reg] << m_rm;
            }
            else
            {
                ht = hde32_table + DELTA_FPU_REG;
                t  = ht[t] << m_reg;
            }
            if (t & 0x80)
                hs->flags |= F_ERROR | F_ERROR_OPCODE;
        }

        if (pref & PRE_LOCK)
        {
            if (m_mod == 3)
            {
                hs->flags |= F_ERROR | F_ERROR_LOCK;
            }
            else
            {
                const uint8_t *table_end;
                auto op = opcode;
                if (hs->opcode2)
                {
                    ht        = hde32_table + DELTA_OP2_LOCK_OK;
                    table_end = ht + DELTA_OP_ONLY_MEM - DELTA_OP2_LOCK_OK;
                }
                else
                {
                    ht        = hde32_table + DELTA_OP_LOCK_OK;
                    table_end = ht + DELTA_OP2_LOCK_OK - DELTA_OP_LOCK_OK;
                    op &= -2;
                }
                for (; ht != table_end; ht++)
                    if (*ht++ == op)
                    {
                        if (!(*ht << m_reg & 0x80))
                            goto no_lock_error;
                        else
                            break;
                    }
                hs->flags |= F_ERROR | F_ERROR_LOCK;
            no_lock_error:;
            }
        }

        if (hs->opcode2)
        {
            switch (opcode)
            {
            case 0x20:
            case 0x22:
                m_mod = 3;
                if (m_reg > 4 || m_reg == 1)
                    goto error_operand;
                else
                    goto no_error_operand;
            case 0x21:
            case 0x23:
                m_mod = 3;
                if (m_reg == 4 || m_reg == 5)
                    goto error_operand;
                else
                    goto no_error_operand;
            }
        }
        else
        {
            switch (opcode)
            {
            case 0x8c:
                if (m_reg > 5)
                    goto error_operand;
                else
                    goto no_error_operand;
            case 0x8e:
                if (m_reg == 1 || m_reg > 5)
                    goto error_operand;
                else
                    goto no_error_operand;
            }
        }

        if (m_mod == 3)
        {
            const uint8_t *table_end;
            if (hs->opcode2)
            {
                ht        = hde32_table + DELTA_OP2_ONLY_MEM;
                table_end = ht + sizeof hde32_table - DELTA_OP2_ONLY_MEM;
            }
            else
            {
                ht        = hde32_table + DELTA_OP_ONLY_MEM;
                table_end = ht + DELTA_OP2_ONLY_MEM - DELTA_OP_ONLY_MEM;
            }
            for (; ht != table_end; ht += 2)
                if (*ht++ == opcode)
                {
                    if (*ht++ & pref && !(*ht << m_reg & 0x80))
                        goto error_operand;
                    else
                        break;
                }
            goto no_error_operand;
        }
        else if (hs->opcode2)
        {
            switch (opcode)
            {
            case 0x50:
            case 0xd7:
            case 0xf7:
                if (pref & (PRE_NONE | PRE_66))
                    goto error_operand;
                break;
            case 0xd6:
                if (pref & (PRE_F2 | PRE_F3))
                    goto error_operand;
                break;
            case 0xc5:
                goto error_operand;
            }
            goto no_error_operand;
        }
        else
            goto no_error_operand;

    error_operand:
        hs->flags |= F_ERROR | F_ERROR_OPERAND;
    no_error_operand:

        c = *p++;
        if (m_reg <= 1)
        {
            if (opcode == 0xf6)
                cflags |= C_IMM8;
            else if (opcode == 0xf7)
                cflags |= C_IMM_P66;
        }

        switch (m_mod)
        {
        case 0:
            if (pref & PRE_67)
            {
                if (m_rm == 6)
                    disp_size = 2;
            }
            else if (m_rm == 5)
                disp_size = 4;
            break;
        case 1:
            disp_size = 1;
            break;
        case 2:
            disp_size = 2;
            if (!(pref & PRE_67))
                disp_size <<= 1;
            break;
        }

        if (m_mod != 3 && m_rm == 4 && !(pref & PRE_67))
        {
            hs->flags |= F_SIB;
            p++;
            hs->sib       = c;
            hs->sib_scale = c >> 6;
            hs->sib_index = (c & 0x3f) >> 3;
            if ((hs->sib_base = c & 7) == 5 && !(m_mod & 1))
                disp_size = 4;
        }

        p--;
        switch (disp_size)
        {
        case 1:
            hs->flags |= F_DISP8;
            hs->disp.disp8 = *p;
            break;
        case 2:
            hs->flags |= F_DISP16;
            hs->disp.disp16 = *reinterpret_cast<uint16_t *>(p);
            break;
        case 4:
            hs->flags |= F_DISP32;
            hs->disp.disp32 = *reinterpret_cast<uint32_t *>(p);
            break;
        }
        p += disp_size;
    }
    else if (pref & PRE_LOCK)
        hs->flags |= F_ERROR | F_ERROR_LOCK;

    if (cflags & C_IMM_P66)
    {
        if (cflags & C_REL32)
        {
            if (pref & PRE_66)
            {
                hs->flags |= F_IMM16 | F_RELATIVE;
                hs->imm.imm16 = *reinterpret_cast<uint16_t *>(p);
                p += 2;
                goto disasm_done;
            }
            goto rel32_ok;
        }
        if (pref & PRE_66)
        {
            hs->flags |= F_IMM16;
            hs->imm.imm16 = *reinterpret_cast<uint16_t *>(p);
            p += 2;
        }
        else
        {
            hs->flags |= F_IMM32;
            hs->imm.imm32 = *reinterpret_cast<uint32_t *>(p);
            p += 4;
        }
    }

    if (cflags & C_IMM16)
    {
        if (hs->flags & F_IMM32)
        {
            hs->flags |= F_IMM16;
            hs->disp.disp16 = *reinterpret_cast<uint16_t *>(p);
        }
        else if (hs->flags & F_IMM16)
        {
            hs->flags |= F_2IMM16;
            hs->disp.disp16 = *reinterpret_cast<uint16_t *>(p);
        }
        else
        {
            hs->flags |= F_IMM16;
            hs->imm.imm16 = *reinterpret_cast<uint16_t *>(p);
        }
        p += 2;
    }
    if (cflags & C_IMM8)
    {
        hs->flags |= F_IMM8;
        hs->imm.imm8 = *p++;
    }

    if (cflags & C_REL32)
    {
    rel32_ok:
        hs->flags |= F_IMM32 | F_RELATIVE;
        hs->imm.imm32 = *reinterpret_cast<uint32_t *>(p);
        p += 4;
    }
    else if (cflags & C_REL8)
    {
        hs->flags |= F_IMM8 | F_RELATIVE;
        hs->imm.imm8 = *p++;
    }

disasm_done:

    if ((hs->len = static_cast<uint8_t>(p - static_cast<uint8_t *>(code))) > 15)
    {
        hs->flags |= F_ERROR | F_ERROR_LENGTH;
        hs->len = 15;
    }

    return hs->len;
}

// ReSharper restore All

static int hde_disasm(void *src, int *reloc_op_offset)
{
    hde32s h;
    if (!hde32_disasm(src, &h))
        return 0;
    if (h.flags & F_ERROR)
        return 0;
    if (reloc_op_offset && h.flags & F_RELATIVE)
    {
        if (h.flags & F_IMM32)
            *reloc_op_offset = h.len - 4;
        else if (h.flags & F_IMM16)
            *reloc_op_offset = h.len - 2;
        else if (h.flags & F_IMM8)
            *reloc_op_offset = h.len;
    }
    return h.len;
}
#elif __has_include(<minhook.h>)
#include <minhook.h>

#ifdef _DEBUG
namespace fd
{
static fmt::string_view mh_status_to_string(MH_STATUS status)
{
    string_view tmp = MH_StatusToString(status);
    return tmp.substr(strlen("MH_"));
}
} // namespace fd

template <>
struct fmt::formatter<MH_STATUS> : formatter<string_view>
{
    auto format(MH_STATUS status, format_context &ctx) const -> format_context::iterator
    {
        return formatter<string_view>::format(fd::mh_status_to_string(status), ctx);
    }
};
#endif
#endif

using std::ranges::all_of;
using std::ranges::for_each;
using std::views::reverse;
using std::views::transform;

namespace fd
{
#ifdef _DEBUG
class simple_action_name
{
    string_view raw_;
    string_view fixed_;

  public:
    consteval simple_action_name(char const *raw, char const *fixed)
        : raw_(raw)
        , fixed_(fixed)
    {
    }

    string_view raw() const
    {
        return raw_;
    }

    string_view get() const
    {
        return fixed_;
    }
};

#define ACTION_NAME(_V_, _RAW_, _FIXED_) constexpr auto _V_ = simple_action_name(_RAW_, _FIXED_);
#else

struct dummy_action_name
{
    static uint8_t get()
    {
        return 0;
    }

    static uint8_t raw()
    {
        return 0;
    }
};

#define ACTION_NAME(_V_, ...) constexpr dummy_action_name _V_;
#endif
namespace act_names
{
ACTION_NAME(enable, "enable", "enabled");
ACTION_NAME(disable, "disable", "disabled");
ACTION_NAME(lazy_enable, "lazy enable", "lazy enabled");
ACTION_NAME(lazy_disable, "lazy disable", "lazy disabled");
ACTION_NAME(sync, "sync", "synced");
ACTION_NAME(create, "create", "created");
ACTION_NAME(destroy, "destroy", "destroyed");
} // namespace act_names

#undef ACTION_NAME

template <typename... Args>
using mh_function = MH_STATUS(WINAPI *)(Args...);

struct hook_info
{
    hook_name name;
    void *target;

    hook_info(hook_info const &) = default;

    hook_info(auto &name, void *target)
        :
#ifdef _DEBUG
        name(name)
        , target(target)
#endif

    {
        ignore_unused(name);
    }
};

template <typename... Args>
static bool mh_action(auto &name, mh_function<LPVOID, Args...> action, hook_info info, Args... args)
{
    auto status = action(info.target, args...);
    if (status != MH_OK)
        log("Unable to {} hook {}: {}", name.raw(), info.name, status);
    else
        log("Hook {} {}", info.name, name.get());
    return status == MH_OK;
}

static bool mh_action(auto &name, mh_function<LPVOID> action)
{
    // ReSharper disable once CppZeroConstantCanBeReplacedWithNullptr
    auto status = action(MH_ALL_HOOKS);
    if (status != MH_OK)
        log("Unable to {} hooks: {}", name.raw(), status);
    else
        log("All hooks {}", name.get());
    return status == MH_OK;
}

static bool mh_action(auto &name, mh_function<> action)
{
    auto status = action();
    if (status != MH_OK)
        log("Unable to {} hook data: {}", name.raw(), status);
    else
        log("Hook data {}", name.get());
    return status == MH_OK;
}

template <typename Fn>
static auto handle_callback(auto &callback, Fn fn)
{
    return [&callback, fn]<class T>(T *obj) -> bool {
        auto ok = std::invoke(fn, obj);
        if (!ok)
            callback(obj);
        return ok;
    };
}

hook_context::~hook_context()
{
    for_each(storage_ | reverse, std::default_delete<basic_hook>());
    mh_action(act_names::destroy, MH_Uninitialize);
}

hook_context::hook_context()
{
    if (!mh_action(act_names::create, MH_Initialize))
        std::destroy_at(&storage_);
}

bool basic_hook_proxy::enable()
{
    return mh_action(act_names::enable, MH_EnableHook, static_cast<basic_hook_data *>(this)->info());
}

bool basic_hook_proxy::disable()
{
    return mh_action(act_names::disable, MH_DisableHook, static_cast<basic_hook_data *>(this)->info());
}

bool basic_hook_lazy_proxy::enable()
{
    return mh_action(act_names::lazy_enable, MH_QueueEnableHook, static_cast<basic_hook_data *>(this)->info());
}

bool basic_hook_lazy_proxy::disable()
{
    return mh_action(act_names::lazy_disable, MH_QueueDisableHook, static_cast<basic_hook_data *>(this)->info());
}

hook_info basic_hook_data::info() const
{
    return {name_, target_};
}
basic_hook_data::basic_hook_data(hook_name name, void *target)
    :
#ifdef _DEBUG
    name_(name)
    ,
#endif
    target_(target)
{
}

basic_hook_lazy *basic_hook_data::lazy()
{
    return this;
}

char const *basic_hook_data::name() const
{
    return name_.data();
}

void *hook_context::create_trampoline(hook_name name, void *target, void *replace)
{
    hook_info info(name, target);
    void *trampoline = nullptr;
    auto created     = mh_action(act_names::create, MH_CreateHook, info, replace, &trampoline);
    /*if (!created && error_handler_)
    {
        error_handler_(nullptr);
    }*/

    ignore_unused<hook_context *>(this, created);

    return trampoline;
}

// void hook_context::set_error_handler(error_handler handler)
//{
//     error_handler_ = std::move(handler);
// }

#if 0
#define RUN_ALL(_ALG_, _STORAGE_, _FN_) \
    (error_handler_ ? _ALG_(_STORAGE_, handle_callback(error_handler_, _FN_)) : _ALG_(_STORAGE_, _FN_))
#else
#define RUN_ALL(_ALG_, _STORAGE_, _FN_) _ALG_(_STORAGE_, _FN_)
#endif

bool hook_context::enable()
{
    return RUN_ALL(all_of, storage_, &basic_hook::enable);
}

bool hook_context::disable()
{
    return RUN_ALL(all_of, storage_ | reverse, &basic_hook::disable);
}

bool hook_context::enable_lazy()
{
    assert(!storage_.empty());
    return RUN_ALL(all_of, storage_ | transform(&basic_hook::lazy), &basic_hook_lazy::enable) &&
           mh_action(act_names::sync, MH_ApplyQueued);
}

bool hook_context::disable_lazy()
{
    assert(!storage_.empty());
    return RUN_ALL(all_of, storage_ | transform(&basic_hook::lazy) | reverse, &basic_hook_lazy::disable) &&
           mh_action(act_names::sync, MH_ApplyQueued);
}

size_t hook_context::size() const
{
    return storage_.size();
}
} // namespace fd