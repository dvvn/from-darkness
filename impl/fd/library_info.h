#pragma once

#include <fd/string.h>

#include <windows.h>
#include <winternl.h>

#include <span>
#include <typeinfo>

/*
server,
client,
engine,
dataCache,
materialSystem,
vstdlib,
vgui2,
vguiMatSurface,
vphysics,
inputSystem,
studioRender,
shaderApiDx9,
serverBrowser
*/

namespace fd
{
    struct library_info;
    struct csgo_library_info;
    struct current_library_info;

    class dos_nt
    {
        void construct(const LDR_DATA_TABLE_ENTRY* ldrEntry);

      public:
        // base address
        IMAGE_DOS_HEADER* dos;
        IMAGE_NT_HEADERS* nt;

        dos_nt(const LDR_DATA_TABLE_ENTRY* ldrEntry);
        explicit dos_nt(library_info info);

        PVOID base() const;

        std::span<uint8_t> read() const;
        std::span<IMAGE_SECTION_HEADER> sections() const;

        template <typename T = uint8_t, typename Q>
        T* map(Q obj) const
        {
            const auto dosAddr = reinterpret_cast<uintptr_t>(dos);
            uintptr_t offset;
            if constexpr (std::is_pointer_v<Q>)
                offset = reinterpret_cast<uintptr_t>(obj);
            else
                offset = static_cast<uintptr_t>(obj);
            return reinterpret_cast<T*>(dosAddr + offset);
        }
    };

    template <class T>
    static constexpr auto _simple_type_name()
    {
        return __FUNCSIG__;
    }

    struct correct_type_name : string_view
    {
        constexpr correct_type_name(const string_view rawName)
            : string_view(rawName.data() + rawName.find('<') + 1, rawName.data() + rawName.rfind('>')) // full name with 'class' or 'struct' prefix, namespace and templates
        {
        }

#ifndef __cpp_lib_string_contains
        constexpr bool contains(const char chr) const
        {
            return find(chr) != npos;
        }
#endif
    };

    class rewrapped_namespaces
    {
        struct simulate_info
        {
            size_t size     = 0;
            bool havePrefix = false;
        };

        template <size_t S>
        struct name_buff
        {
            char buff[S];
        };

        string_view prefix_;
        string_view name_;

      public:
        constexpr rewrapped_namespaces(const string_view correctedName)
        {
            const auto spacePos = correctedName.find(' ');
            if (spacePos == correctedName.npos)
                name_ = correctedName;
            else
            {
                prefix_ = correctedName.substr(0, spacePos);
                name_   = correctedName.substr(spacePos + 1);
            }
        }

        constexpr string_view name() const
        {
            return name_;
        }

        constexpr bool is_class() const
        {
            return prefix_ == "class";
        }

        constexpr bool is_struct() const
        {
            return prefix_ == "struct";
        }

        constexpr size_t calc_size() const
        {
            const auto namespacesChars = std::ranges::count(name_, ':');
            // ReSharper disable CppVariableCanBeMadeConstexpr
            const size_t prefixSize    = 0 /*prefix_.size()*/;
            const size_t postfixSize   = 0 /*2*/;
            // ReSharper restore CppVariableCanBeMadeConstexpr
            return prefixSize + name_.size() - namespacesChars / 2 + postfixSize;
        }

        template <size_t S>
        constexpr name_buff<S> get() const
        {
            name_buff<S> ret{};

            auto itr       = name_.data();
            // copy prefix if exist
            /* if (auto space_pos = name_.find(' '); space_pos != name_.npos)
            {
                ++space_pos;
                std::copy_n(itr, space_pos, ret.buff);
                itr += space_pos;
            } */
            char* buffEnd  = ret.buff + S;
            // create postfix
            /* *--buff_end = '@';
             *--buff_end = '@'; */
            const auto end = name_.data() + name_.size();
            for (;;)
            {
                const auto wordEnd  = std::find(itr, end, ':');
                const auto wordSize = std::distance(itr, wordEnd);
                std::copy_n(itr, wordSize, buffEnd -= wordSize);
                if (wordEnd == end)
                    break;
                *--buffEnd = '@';
                itr += wordSize + 2;
            }

            return ret;
        }
    };

    library_info find_library(wstring_view name, bool notify = true);
    PVOID wait_for_library(wstring_view name);

    struct library_info
    {
        using pointer   = const LDR_DATA_TABLE_ENTRY*;
        using reference = const LDR_DATA_TABLE_ENTRY&;

      private:
        pointer entry_;

      public:
        library_info();
        library_info(pointer entry);
        library_info(wstring_view name, bool wait, bool notify = true); // todo: delay or cancel
        library_info(const IMAGE_DOS_HEADER* baseAddress, bool notify = true);

        bool is_root() const;
        bool unload() const;

        pointer get() const;
        pointer operator->() const;
        reference operator*() const;

        explicit operator bool() const;

        wstring_view path() const;
        wstring_view name() const;

        void log_class_info(string_view rawName, const void* addr) const;

        template <class T>
        void log_class_info(const T* addr) const
        {
            constexpr correct_type_name name(_simple_type_name<T>());
            log_class_info(name, addr);
        }

        void* find_export(string_view name, bool notify = true) const;
        IMAGE_SECTION_HEADER* find_section(string_view name, bool notify = true) const;
        void* find_signature(string_view sig, bool notify = true) const;

      private:
        void* find_vtable_class(string_view name, bool notify) const;
        void* find_vtable_struct(string_view name, bool notify) const;
        void* find_vtable_unknown(string_view name, bool notify) const;

      public:
        void* find_vtable(string_view name, bool notify = true) const;
        void* find_vtable(const std::type_info& info, bool notify = true) const;

        template <class T>
        T* find_vtable(const bool notify = true) const
        {
            constexpr correct_type_name name(_simple_type_name<T>());
            void* ptr;
            if constexpr (name.contains('<')) // templates currently unsupported
            {
                ptr = find_vtable(typeid(T), notify);
            }
            else if constexpr (name.contains(':'))
            {
                constexpr rewrapped_namespaces corrName(name);
                constexpr auto size = corrName.calc_size();
                constexpr auto ret  = corrName.get<size>();
                constexpr string_view result(ret.buff, size);
                // const string_view test = typeid(T).raw_name();
                if constexpr (corrName.is_class())
                    ptr = find_vtable_class(result, notify);
                else if constexpr (corrName.is_struct())
                    ptr = find_vtable_struct(result, notify);
                else
                {
                    // ptr = find_vtable_unknown(result, notify);
                    static_assert(std::_Always_false<T>, "Wrong prefix!");
                }
            }
            else
            {
                ptr = find_vtable(name, notify);
            }
            return static_cast<T*>(ptr);
        }
    };

    struct csgo_library_info : library_info
    {
        using library_info::library_info;

        void* find_interface(string_view name, bool notify = true) const;
        void* find_interface(const void* createInterfaceFn, string_view name, bool notify = true) const;
    };

    struct current_library_info : library_info
    {
        current_library_info(bool notify = true);
    };

    extern HMODULE CurrentLibraryHandle;
} // namespace fd