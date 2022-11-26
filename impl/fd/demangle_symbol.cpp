#include "demangle_symbol.h"

#include <memory>

typedef void*(__cdecl* allocation_function)(size_t);
typedef void(__cdecl* free_function)(void*);
extern "C" char* __cdecl __unDName(char* outputString, const char* name, int maxStringLength, allocation_function pAlloc, free_function pFree, uint16_t disableFlags);

namespace fd
{
    string demangle_symbol(const char* mangled_name)
    {
        constexpr allocation_function alloc = [](size_t size) -> void* {
            return new uint8_t[size];
        };
        constexpr free_function free_f = [](void* p) {
            auto chr = static_cast<uint8_t*>(p);
            delete[] chr;
        };

        const std::unique_ptr<char> name(__unDName(nullptr, mangled_name + 1, 0, alloc, free_f, 0x2800 /*UNDNAME_32_BIT_DECODE | UNDNAME_TYPE_ONLY*/));
        return name.get();
    }
} // namespace fd
