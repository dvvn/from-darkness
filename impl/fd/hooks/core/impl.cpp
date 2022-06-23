module;

#include <fd/core/assert.h>

#include <Windows.h>

#include <functional>
#include <string>

module fd.hook;
import fd.logger;
import fd.mem_block;

#pragma region hde

// hde - header ------------------------

#define F_MODRM         0x00000001
#define F_SIB           0x00000002
#define F_IMM8          0x00000004
#define F_IMM16         0x00000008
#define F_IMM32         0x00000010
#define F_DISP8         0x00000020
#define F_DISP16        0x00000040
#define F_DISP32        0x00000080
#define F_RELATIVE      0x00000100
#define F_2IMM16        0x00000800
#define F_ERROR         0x00001000
#define F_ERROR_OPCODE  0x00002000
#define F_ERROR_LENGTH  0x00004000
#define F_ERROR_LOCK    0x00008000
#define F_ERROR_OPERAND 0x00010000
#define F_PREFIX_REPNZ  0x01000000
#define F_PREFIX_REPX   0x02000000
#define F_PREFIX_REP    0x03000000
#define F_PREFIX_66     0x04000000
#define F_PREFIX_67     0x08000000
#define F_PREFIX_LOCK   0x10000000
#define F_PREFIX_SEG    0x20000000
#define F_PREFIX_ANY    0x3f000000

#define PREFIX_SEGMENT_CS   0x2e
#define PREFIX_SEGMENT_SS   0x36
#define PREFIX_SEGMENT_DS   0x3e
#define PREFIX_SEGMENT_ES   0x26
#define PREFIX_SEGMENT_FS   0x64
#define PREFIX_SEGMENT_GS   0x65
#define PREFIX_LOCK         0xf0
#define PREFIX_REPNZ        0xf2
#define PREFIX_REPX         0xf3
#define PREFIX_OPERAND_SIZE 0x66
#define PREFIX_ADDRESS_SIZE 0x67

#pragma pack(push, 1)

typedef struct
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
} hde32s;

#pragma pack(pop)

// hde - header(table) ------------------------

#define C_NONE    0x00
#define C_MODRM   0x01
#define C_IMM8    0x02
#define C_IMM16   0x04
#define C_IMM_P66 0x10
#define C_REL8    0x20
#define C_REL32   0x40
#define C_GROUP   0x80
#define C_ERROR   0xff

#define PRE_ANY  0x00
#define PRE_NONE 0x01
#define PRE_F2   0x02
#define PRE_F3   0x04
#define PRE_66   0x08
#define PRE_67   0x10
#define PRE_LOCK 0x20
#define PRE_SEG  0x40
#define PRE_ALL  0xff

#define DELTA_OPCODES      0x4a
#define DELTA_FPU_REG      0xf1
#define DELTA_FPU_MODRM    0xf8
#define DELTA_PREFIXES     0x130
#define DELTA_OP_LOCK_OK   0x1a1
#define DELTA_OP2_LOCK_OK  0x1b9
#define DELTA_OP_ONLY_MEM  0x1cb
#define DELTA_OP2_ONLY_MEM 0x1da

static uint8_t hde32_table[] = {
    0xa3, 0xa8, 0xa3, 0xa8, 0xa3, 0xa8, 0xa3, 0xa8, 0xa3, 0xa8, 0xa3, 0xa8, 0xa3, 0xa8, 0xa3, 0xa8, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xac, 0xaa, 0xb2, 0xaa, 0x9f,
    0x9f, 0x9f, 0x9f, 0xb5, 0xa3, 0xa3, 0xa4, 0xaa, 0xaa, 0xba, 0xaa, 0x96, 0xaa, 0xa8, 0xaa, 0xc3, 0xc3, 0x96, 0x96, 0xb7, 0xae, 0xd6, 0xbd, 0xa3, 0xc5, 0xa3, 0xa3, 0x9f, 0xc3,
    0x9c, 0xaa, 0xaa, 0xac, 0xaa, 0xbf, 0x03, 0x7f, 0x11, 0x7f, 0x01, 0x7f, 0x01, 0x3f, 0x01, 0x01, 0x90, 0x82, 0x7d, 0x97, 0x59, 0x59, 0x59, 0x59, 0x59, 0x7f, 0x59, 0x59, 0x60,
    0x7d, 0x7f, 0x7f, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x9a, 0x88, 0x7d, 0x59, 0x50, 0x50, 0x50, 0x50, 0x59, 0x59, 0x59, 0x59, 0x61, 0x94,
    0x61, 0x9e, 0x59, 0x59, 0x85, 0x59, 0x92, 0xa3, 0x60, 0x60, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x9f, 0x01, 0x03, 0x01, 0x04, 0x03, 0xd5, 0x03,
    0xcc, 0x01, 0xbc, 0x03, 0xf0, 0x10, 0x10, 0x10, 0x10, 0x50, 0x50, 0x50, 0x50, 0x14, 0x20, 0x20, 0x20, 0x20, 0x01, 0x01, 0x01, 0x01, 0xc4, 0x02, 0x10, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0xc0, 0xc2, 0x10, 0x11, 0x02, 0x03, 0x11, 0x03, 0x03, 0x04, 0x00, 0x00, 0x14, 0x00, 0x02, 0x00, 0x00, 0xc6, 0xc8, 0x02, 0x02, 0x02, 0x02, 0x00, 0x00, 0xff, 0xff,
    0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xca, 0x01, 0x01, 0x01, 0x00, 0x06, 0x00, 0x04, 0x00, 0xc0, 0xc2, 0x01, 0x01, 0x03, 0x01, 0xff, 0xff, 0x01, 0x00, 0x03, 0xc4, 0xc4, 0xc6,
    0x03, 0x01, 0x01, 0x01, 0xff, 0x03, 0x03, 0x03, 0xc8, 0x40, 0x00, 0x0a, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x33, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
    0xbf, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
    0x00, 0xbf, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0xff, 0x4a, 0x4a, 0x4a, 0x4a, 0x4b, 0x52, 0x4a, 0x4a, 0x4a, 0x4a, 0x4f, 0x4c, 0x4a, 0x4a, 0x4a,
    0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x55, 0x45, 0x40, 0x4a, 0x4a, 0x4a, 0x45, 0x59, 0x4d, 0x46, 0x4a, 0x5d, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a,
    0x4a, 0x4a, 0x61, 0x63, 0x67, 0x4e, 0x4a, 0x4a, 0x6b, 0x6d, 0x4a, 0x4a, 0x45, 0x6d, 0x4a, 0x4a, 0x44, 0x45, 0x4a, 0x4a, 0x00, 0x00, 0x00, 0x02, 0x0d, 0x06, 0x06, 0x06, 0x06,
    0x0e, 0x00, 0x00, 0x00, 0x00, 0x06, 0x06, 0x06, 0x00, 0x06, 0x06, 0x02, 0x06, 0x00, 0x0a, 0x0a, 0x07, 0x07, 0x06, 0x02, 0x05, 0x05, 0x02, 0x02, 0x00, 0x00, 0x04, 0x04, 0x04,
    0x04, 0x00, 0x00, 0x00, 0x0e, 0x05, 0x06, 0x06, 0x06, 0x01, 0x06, 0x00, 0x00, 0x08, 0x00, 0x10, 0x00, 0x18, 0x00, 0x20, 0x00, 0x28, 0x00, 0x30, 0x00, 0x80, 0x01, 0x82, 0x01,
    0x86, 0x00, 0xf6, 0xcf, 0xfe, 0x3f, 0xab, 0x00, 0xb0, 0x00, 0xb1, 0x00, 0xb3, 0x00, 0xba, 0xf8, 0xbb, 0x00, 0xc0, 0x00, 0xc1, 0x00, 0xc7, 0xbf, 0x62, 0xff, 0x00, 0x8d, 0xff,
    0x00, 0xc4, 0xff, 0x00, 0xc5, 0xff, 0x00, 0xff, 0xff, 0xeb, 0x01, 0xff, 0x0e, 0x12, 0x08, 0x00, 0x13, 0x09, 0x00, 0x16, 0x08, 0x00, 0x17, 0x09, 0x00, 0x2b, 0x09, 0x00, 0xae,
    0xff, 0x07, 0xb2, 0xff, 0x00, 0xb4, 0xff, 0x00, 0xb5, 0xff, 0x00, 0xc3, 0x01, 0x00, 0xc7, 0xff, 0xbf, 0xe7, 0x08, 0x00, 0xf0, 0x02, 0x00
};

// hde - source ------------------------

static unsigned int hde32_disasm(const void* code, hde32s* hs)
{
    uint8_t x, c, *p = (uint8_t*)code, cflags, opcode, pref = 0;
    uint8_t *ht = hde32_table, m_mod, m_reg, m_rm, disp_size = 0;

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

    hs->flags = (uint32_t)pref << 23;

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

    opcode = c;
    cflags = ht[ht[opcode / 4] + (opcode % 4)];

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
        uint16_t t;
        t      = *(uint16_t*)(ht + (cflags & 0x7f));
        cflags = (uint8_t)t;
        x      = (uint8_t)(t >> 8);
    }

    if (hs->opcode2)
    {
        ht = hde32_table + DELTA_PREFIXES;
        if (ht[ht[opcode / 4] + (opcode % 4)] & pref)
            hs->flags |= F_ERROR | F_ERROR_OPCODE;
    }

    if (cflags & C_MODRM)
    {
        hs->flags |= F_MODRM;
        hs->modrm = c = *p++;
        hs->modrm_mod = m_mod = c >> 6;
        hs->modrm_rm = m_rm = c & 7;
        hs->modrm_reg = m_reg = (c & 0x3f) >> 3;

        if (x && ((x << m_reg) & 0x80))
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
                uint8_t *table_end, op = opcode;
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
                        if (!((*ht << m_reg) & 0x80))
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
            uint8_t* table_end;
            if (hs->opcode2)
            {
                ht        = hde32_table + DELTA_OP2_ONLY_MEM;
                table_end = ht + sizeof(hde32_table) - DELTA_OP2_ONLY_MEM;
            }
            else
            {
                ht        = hde32_table + DELTA_OP_ONLY_MEM;
                table_end = ht + DELTA_OP2_ONLY_MEM - DELTA_OP_ONLY_MEM;
            }
            for (; ht != table_end; ht += 2)
                if (*ht++ == opcode)
                {
                    if (*ht++ & pref && !((*ht << m_reg) & 0x80))
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
            hs->disp.disp16 = *(uint16_t*)p;
            break;
        case 4:
            hs->flags |= F_DISP32;
            hs->disp.disp32 = *(uint32_t*)p;
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
                hs->imm.imm16 = *(uint16_t*)p;
                p += 2;
                goto disasm_done;
            }
            goto rel32_ok;
        }
        if (pref & PRE_66)
        {
            hs->flags |= F_IMM16;
            hs->imm.imm16 = *(uint16_t*)p;
            p += 2;
        }
        else
        {
            hs->flags |= F_IMM32;
            hs->imm.imm32 = *(uint32_t*)p;
            p += 4;
        }
    }

    if (cflags & C_IMM16)
    {
        if (hs->flags & F_IMM32)
        {
            hs->flags |= F_IMM16;
            hs->disp.disp16 = *(uint16_t*)p;
        }
        else if (hs->flags & F_IMM16)
        {
            hs->flags |= F_2IMM16;
            hs->disp.disp16 = *(uint16_t*)p;
        }
        else
        {
            hs->flags |= F_IMM16;
            hs->imm.imm16 = *(uint16_t*)p;
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
        hs->imm.imm32 = *(uint32_t*)p;
        p += 4;
    }
    else if (cflags & C_REL8)
    {
        hs->flags |= F_IMM8 | F_RELATIVE;
        hs->imm.imm8 = *p++;
    }

disasm_done:

    if ((hs->len = (uint8_t)(p - (uint8_t*)code)) > 15)
    {
        hs->flags |= F_ERROR | F_ERROR_LENGTH;
        hs->len = 15;
    }

    return (unsigned int)hs->len;
}

#pragma endregion

#pragma pack(push, 1)

// 8-bit relative jump.
struct JMP_REL_SHORT
{
    uint8_t opcode; // EB xx: JMP +2+xx
    uint8_t operand;
};

// 32-bit direct relative jump/call.
struct JMP_REL
{
    uint8_t opcode;   // E9/E8 xxxxxxxx: JMP/CALL +5+xxxxxxxx
    uint32_t operand; // Relative destination address
};

using CALL_REL = JMP_REL;

// 64-bit indirect absolute jump.
struct JMP_ABS
{
    uint8_t opcode0; // FF25 00000000: JMP [+6]
    uint8_t opcode1;
    uint32_t dummy;
    uint64_t address; // Absolute destination address
};

// 64-bit indirect absolute call.
struct CALL_ABS
{
    uint8_t opcode0; // FF15 00000002: CALL [+6]
    uint8_t opcode1;
    uint32_t dummy0;
    uint8_t dummy1; // EB 08:         JMP +10
    uint8_t dummy2;
    uint64_t address; // Absolute destination address
};

// 32-bit direct relative conditional jumps.
struct JCC_REL
{
    uint8_t opcode0; // 0F8* xxxxxxxx: J** +6+xxxxxxxx
    uint8_t opcode1;
    uint32_t operand; // Relative destination address
};

// 64-bit indirect absolute conditional jumps that x64 lacks.
struct JCC_ABS
{
    uint8_t opcode; // 7* 0E:         J** +16
    uint8_t dummy0;
    uint8_t dummy1; // FF25 00000000: JMP [+6]
    uint8_t dummy2;
    uint32_t dummy3;
    uint64_t address; // Absolute destination address
};

#pragma pack(pop)

//----------------------

hook_entry::hook_entry() = default;

hook_entry::~hook_entry()
{
    disable();
}

hook_entry::hook_entry(hook_entry&& other)            = default;
hook_entry& hook_entry::operator=(hook_entry&& other) = default;

template <typename T>
static fd::mem_block _To_mem_block(T* ptr)
{
    return { reinterpret_cast<uint8_t*>(ptr), std::max(sizeof(uintptr_t), sizeof(T)) };
}

static fd::mem_block _To_mem_block(void* ptr)
{
    return { reinterpret_cast<uint8_t*>(ptr), sizeof(uintptr_t) };
}

template <typename T>
static fd::mem_block _To_mem_block(T& rng)
{
    return { reinterpret_cast<uint8_t*>(rng.data()), rng.size() * sizeof(T::value_type) };
}

// template<class T>
// static bool _Have_flags(T && obj, DWORD flags)
//{
//	return _To_mem_block(obj).have_flags(flags);
// }

bool hook_entry::create()
{
    if (created())
        return 0;
    if (!target_)
        return 0;
    if (!detour_)
        return 0;
    if (target_ == detour_)
        return /*hook_status::ERROR_UNSUPPORTED_FUNCTION*/ 0;

    if (!_To_mem_block(target_).executable())
        return /*hook_status::ERROR_NOT_EXECUTABLE*/ 0;
    if (!_To_mem_block(detour_).executable())
        return /*hook_status::ERROR_NOT_EXECUTABLE*/ 0;

#if 0 // x64
    CALL_ABS call = {
        0xFF,
        0x15,
        0x00000002, // FF15 00000002: CALL [RIP+8]
        0xEB,
        0x08,                 // EB 08:         JMP +10
        0x0000000000000000ULL // Absolute destination address
    };
    JMP_ABS jmp = {
        0xFF, 0x25, 0x00000000, // FF25 00000000: JMP [RIP+6]
        0x0000000000000000ULL   // Absolute destination address
    };
    JCC_ABS jcc = {
        0x70,
        0x0E, // 7* 0E:         J** +16
        0xFF,
        0x25,
        0x00000000,           // FF25 00000000: JMP [RIP+6]
        0x0000000000000000ULL // Absolute destination address
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

    uint8_t old_pos    = 0;
    uint8_t new_pos    = 0;
    ULONG_PTR jmp_dest = 0;     // Destination address of an internal jump.
    bool finished      = false; // Is the function completed?
#if 0                           // x64
    uint8_t instBuf[16];
#endif

    do
    {
#if 0 // x64
        hde64s hs;
#else
        hde32s hs;
#endif
        uint8_t copy_size = 0;
        ULONG_PTR new_inst;
        const auto old_inst = reinterpret_cast<ULONG_PTR>(target_) + old_pos;
        // const address old_inst = old_pos ? address(target_) + old_pos : target_;

        // ReSharper disable once CppInconsistentNaming
        const auto _Set_copy_size = [&](uint8_t size) {
            if (copy_size < size)
            {
                const auto size_diff = size - copy_size;
                /*const auto pad       = [&]( )-> size_t
                {
                    const auto     estimate_size = tr.size( ) + size_diff;
                    constexpr auto delim         = sizeof(ULONG_PTR);
                    if (estimate_size < delim)
                        return delim - estimate_size;
                    if (estimate_size > delim)
                        return estimate_size % delim;
                    return 0;
                }( );*/
                trampoline_.insert(std::next(trampoline_.begin(), new_pos + copy_size), size_diff /*+ pad*/, -1);

                new_inst = reinterpret_cast<ULONG_PTR>(trampoline_.data()) + new_pos;
            }
            copy_size = size;
        };

#if 0 // x64
        _Set_copy_size(hde64_disasm(reinterpret_cast<void*>(old_inst), &hs));
#else
        _Set_copy_size(hde32_disasm(reinterpret_cast<void*>(old_inst), &hs));
#endif

        if (hs.flags & F_ERROR)
            return false;

        auto copy_src = reinterpret_cast<void*>(old_inst);
        if (old_pos >= sizeof(JMP_REL))
        {
            copy_src = &jmp;
            _Set_copy_size(sizeof(decltype(jmp)));

            // The trampoline function is long enough.
            // Complete the function with the jump to the target_ function.
#if 0 // x64
            jmp.address = old_inst;
#else
            jmp.operand  = static_cast<uint32_t>(old_inst - (new_inst + copy_size));
#endif

            finished = true;
        }
#if 0 // x64
        else if ((hs.modrm & 0xC7) == 0x05)
        {
            // Instructions using RIP relative addressing. (ModR/M = 00???101B)

            // Modify the RIP relative address.
            uint32_t* pRelAddr;

            std::memcpy(instBuf, (LPBYTE)old_inst, copy_size);

            copy_src = instBuf;

            // Relative address is stored at (instruction length - immediate value length - 4).
            pRelAddr  = (uint32_t*)(instBuf + hs.len - ((hs.flags & 0x3C) >> 2) - 4);
            *pRelAddr = static_cast<uint32_t>(old_inst + hs.len + static_cast<INT32>(hs.disp.disp32) - (new_inst + hs.len));

            // Complete the function if JMP (FF /4).
            if (hs.opcode == 0xFF && hs.modrm_reg == 4)
                finished = true;
        }
#endif
        else if (hs.opcode == 0xE8)
        {
            copy_src = &call;
            _Set_copy_size(sizeof(decltype(call)));

            // Direct relative CALL
            const auto dest = old_inst + hs.len + static_cast<INT32>(hs.imm.imm32);
#if 0 // x64
            call.address = dest;
#else
            call.operand = static_cast<uint32_t>(dest - (new_inst + copy_size));
#endif
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
            if (reinterpret_cast<ULONG_PTR>(target_) <= dest && dest < reinterpret_cast<ULONG_PTR>(target_) + sizeof(JMP_REL))
            {
                if (jmp_dest < dest)
                    jmp_dest = dest;
            }
            else
            {
                copy_src = &jmp;
                _Set_copy_size(sizeof(decltype(jmp)));

#if 0 // x64
                jmp.address = dest;
#else
                jmp.operand = static_cast<uint32_t>(dest - (new_inst + copy_size));
#endif

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
            if (reinterpret_cast<ULONG_PTR>(target_) <= dest && dest < reinterpret_cast<ULONG_PTR>(target_) + sizeof(JMP_REL))
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
                copy_src = &jcc;
                _Set_copy_size(sizeof(decltype(jcc)));

                const uint8_t cond = (hs.opcode != 0x0F ? hs.opcode : hs.opcode2) & 0x0F;
#if 0 // x64
      //  Invert the condition in x64 mode to simplify the conditional jump logic.
                jcc.opcode  = 0x71 ^ cond;
                jcc.address = dest;
#else
                jcc.opcode1 = 0x80 | cond;
                jcc.operand = static_cast<uint32_t>(dest - (new_inst + copy_size));
#endif
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

#if 0
		// Trampoline function is too large.
		if (new_pos + copy_size > buffer_size())
			return false;

		// Trampoline function has too many instructions.
		if (ct.ips_count >= /*ct.old_ips.size( )*/sizeof(ips_type))
			return false;
#endif

#ifdef DHOOKS_ENTRY_STORE_IPS
        old_ips_.push_back(old_pos);
        new_ips_.push_back(new_pos);
#endif

        std::copy_n(static_cast<uint8_t*>(copy_src), copy_size, reinterpret_cast<uint8_t*>(new_inst));
        // std::memcpy(reinterpret_cast<void*>(new_inst), copy_src, copy_size);

        new_pos += copy_size;
        old_pos += hs.len;
    }
    while (!finished);

    const auto target_ptr = static_cast<uint8_t*>(target_);

    // Is there enough place for a long jump?
    if (old_pos < sizeof(JMP_REL) && !fd::mem_block(target_ptr + old_pos, sizeof(JMP_REL) - old_pos).code_padding())
    {
        // Is there enough place for a short jump?
        if (old_pos < sizeof(JMP_REL_SHORT) && !fd::mem_block(target_ptr + old_pos, sizeof(JMP_REL_SHORT) - old_pos).code_padding())
            return false;

        const fd::mem_block target_rel(target_ptr - sizeof(JMP_REL), sizeof(JMP_REL));

        // Can we place the long jump above the function?
        if (!target_rel.executable())
            return false;
        if (!target_rel.code_padding())
            return false;

        patch_above_ = true;
    }

#if 0 // x64
    // Create a relay function.
    jmp.address = reinterpret_cast<ULONG_PTR>(ct.pDetour);

    ct.pRelay = static_cast<LPBYTE>(trampoline_.data()) + new_pos;
    WIP std::memcpy(ct.pRelay, &jmp, sizeof jmp);

    detour_ = ct.pRelay;
#endif

    // correct trampoline memory access
    if (!fd::mem_block(trampoline_.data(), trampoline_.size()).have_flags(PAGE_EXECUTE_READWRITE))
    {
        FD_ASSERT(!trampoline_protection_.has_value(), "Trampoline memory protection already fixed");
        trampoline_protection_ = { trampoline_.data(), trampoline_.size(), PAGE_EXECUTE_READWRITE };
        FD_ASSERT(trampoline_protection_.has_value(), "Unable to fix trampoline memory protection");
    }

    // Back up the target function.
    FD_ASSERT(target_backup_.empty());
    if (patch_above_)
        target_backup_.assign(target_ptr - sizeof(JMP_REL), target_ptr /*- sizeof(JMP_REL) + sizeof(JMP_REL)*/ + sizeof(JMP_REL_SHORT));
    else
        target_backup_.assign(target_ptr, target_ptr + sizeof(JMP_REL));

    return true;
}

bool hook_entry::created() const
{
    return !trampoline_.empty();
}

bool hook_entry::enabled() const
{
    return enabled_;
}

class prepared_memory_block : public fd::mem_block
{
    fd::mem_protect protect_;

  public:
    ~prepared_memory_block()
    {
        if (this->empty())
            return;

        protect_.restore();
        FlushInstructionCache(GetCurrentProcess(), this->data(), this->size());
    }

    prepared_memory_block(const prepared_memory_block&) = delete;

    prepared_memory_block(void* target, bool patch_above)
    {
        FD_ASSERT(target != nullptr);

        auto target_ptr      = static_cast<uint8_t*>(target);
        auto target_ptr_size = sizeof(JMP_REL);

        if (patch_above)
        {
            target_ptr -= sizeof(JMP_REL);
            target_ptr_size += sizeof(JMP_REL_SHORT);
        }

        *static_cast<fd::mem_block*>(this) = { target_ptr, target_ptr_size };

        // todo: wait if not readable
        //-----

        if (!this->have_flags(PAGE_EXECUTE_READWRITE))
        {
            protect_ = { target_ptr, target_ptr_size, PAGE_EXECUTE_READWRITE };
            FD_ASSERT(protect_.has_value());
        }
    }
};

bool hook_entry::enable()
{
    if (enabled_)
        return false;
    const prepared_memory_block block(target_, patch_above_);
    if (block.empty())
        return false;

    const auto jmp_rel = reinterpret_cast<JMP_REL*>(block.data());
    jmp_rel->opcode    = 0xE9;
    jmp_rel->operand   = static_cast<UINT32>(static_cast<LPBYTE>(detour_) - (block.data() + sizeof(JMP_REL)));
    if (patch_above_)
    {
        const auto short_jmp = static_cast<JMP_REL_SHORT*>(target_);
        short_jmp->opcode    = 0xEB;
        short_jmp->operand   = static_cast<UINT8>(0 - (sizeof(JMP_REL_SHORT) + sizeof(JMP_REL)));
    }

    enabled_ = true;
    return true;
}

bool hook_entry::disable()
{
    if (!enabled_)
        return false;
    const prepared_memory_block mem(target_, patch_above_);
    if (mem.empty())
        return false;

    std::copy(target_backup_.begin(), target_backup_.end(), mem.data());
    enabled_ = false;
    return true;
}

void* hook_entry::get_original_method() const
{
    FD_ASSERT(created());
    auto ptr = trampoline_.data();
    return (void*)ptr;
}

void* hook_entry::get_target_method() const
{
    return target_;
}

void* hook_entry::get_replace_method() const
{
    return detour_;
}

void hook_entry::set_target_method(void* getter)
{
    FD_ASSERT(target_ == nullptr);
    FD_ASSERT(getter != nullptr);
    target_ = getter;
}

void hook_entry::set_replace_method(void* getter)
{
    FD_ASSERT(detour_ == nullptr);
    FD_ASSERT(getter != nullptr);
    detour_ = getter;
}

//----------------

hook_impl::~hook_impl()
{
    // purecall here
    // hook_impl::disable( );
}

hook_impl::hook_impl() = default;

template <typename M>
static void _Log(const hook_impl* h, const M& msg)
{
    fd::logger("{}: {}", std::bind_front(&hook_impl::name, h), msg);
}

bool hook_impl::enable()
{
    if (!entry_.created() && !entry_.create())
    {
        _Log(this, "created error!");
        return false;
    }
    if (!entry_.enable())
    {
        _Log(this, [this] {
            return entry_.enabled() ? "already hooked" : "enable error!";
        });
        return false;
    }
    _Log(this, "hooked");
    return true;
}

bool hook_impl::disable()
{
    const auto ok = entry_.disable();
    _Log(this, [ok, this] {
        if (ok)
            return "unhooked";
        if (!entry_.enabled())
            return "already unhooked";
        if (entry_.created())
            return "unhook error!";
        return "not created!";
    });
    return ok;
}

bool hook_impl::initialized() const
{
    return entry_.get_target_method() && entry_.get_replace_method();
}

bool hook_impl::active() const
{
    return entry_.enabled();
}

void* hook_impl::get_original_method() const
{
    return entry_.get_original_method();
}

void hook_impl::init(const function_getter target, const function_getter replace)
{
    entry_.set_target_method(target);
    entry_.set_replace_method(replace);
}

#if 0
std::string hook_impl::name( ) const
{
	std::string ret;
	if(this->is_static( ))
	{
		const auto fn_name = dynamic_cast<const static_base*>(this)->function_name( );
		ret = {fn_name.begin( ),fn_name.end( )};
	}
	else
	{
		const auto base = dynamic_cast<const class_base*>(this);
		const auto class_name = base->class_name( );
		const auto fn_name = base->function_name( );
		ret.reserve(class_name.size( ) + 2 + fn_name.size( ));
		ret += class_name;
		ret += ':';
		ret += ':';
		ret += fn_name;
	}
	return ret;
}
#endif
