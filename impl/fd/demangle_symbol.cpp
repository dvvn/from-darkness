#include "demangle_symbol.h"

#include <memory>

using allocation_function = void*(__cdecl*)(size_t);
using free_function       = void(__cdecl*)(void*);
// ReSharper disable once CppInconsistentNaming
extern "C" char* __cdecl __unDName(char* outputString, const char* name, int maxStringLength, allocation_function pAlloc, free_function pFree, uint16_t disableFlags);

namespace fd
{
    string demangle_symbol(const char* mangledName)
    {
        // ReSharper disable CppInconsistentNaming
        constexpr allocation_function alloc_fn = [](const size_t size) -> void* {
            return new uint8_t[size];
        };
        constexpr free_function free_fn = [](void* p) {
            const auto chr = static_cast<uint8_t*>(p);
            delete[] chr;
        };
        // ReSharper restore CppInconsistentNaming

        const std::unique_ptr<char> name(__unDName(nullptr, mangledName + 1, 0, alloc_fn, free_fn, 0x2800 /*UNDNAME_32_BIT_DECODE | UNDNAME_TYPE_ONLY*/));
        return name.get();
    }
} // namespace fd
