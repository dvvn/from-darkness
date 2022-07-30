module;

#include <fd/utility.h>

#include <fd/rt_modules/winapi_fwd.h>

#include <memory>

export module fd.rt_modules;
import :find_library;
import :find_export;
import :find_section;
import :find_vtable;
import :find_signature;
import :find_csgo_interface;
export import fd.address;
export import fd.type_name;
import fd.chars_cache;

using fd::basic_address;

struct only_true
{
    only_true(const bool val);
};

void on_class_found(const LDR_DATA_TABLE_ENTRY* entry, const fd::string_view raw_name, const void* addr);

namespace fd
{
    export using ::on_class_found;

    struct any_module_base
    {
        virtual ~any_module_base() = default;

        virtual LDR_DATA_TABLE_ENTRY* data() const = 0;
        virtual bool loaded() const                = 0;

        LDR_DATA_TABLE_ENTRY* operator->() const
        {
            return data();
        }

        LDR_DATA_TABLE_ENTRY& operator*() const
        {
            return *data();
        }

        void log(const fd::string_view object_name, const void* addr) const;
    };

    template <size_t UniqueNum>
    struct any_module : any_module_base
    {
        template <chars_cache ExpName>
        basic_address<void> find_export() const
        {
            static const auto found = fd::find_export(this->data(), ExpName);
            return found;
        }

        template <chars_cache Sig>
        basic_address<void> find_signature() const
        {
            static const auto found = fd::find_signature(this->data(), Sig);
            return found;
        }

        template <chars_cache Name>
        void* find_vtable() const
        {
            static const auto found = fd::find_vtable(this->data(), Name);
            return found;
        }

        template <class T>
        T* find_vtable() const
        {
            static const auto found = fd::find_vtable<T>(this->data());
            return found;
        }
    };

    template <chars_cache Name>
    class rt_module final : public any_module<_Hash_object(Name)>
    {
        static LDR_DATA_TABLE_ENTRY*& _Data()
        {
            static LDR_DATA_TABLE_ENTRY* val = nullptr;
            return val;
        }

        static bool _Loaded(const bool log_error)
        {
            auto& ptr = _Data();
            if (!ptr)
            {
                ptr = find_library(Name, false);
                if (ptr || log_error)
                    on_library_found(Name, ptr);
            }
            return ptr != nullptr;
        }

      public:
        LDR_DATA_TABLE_ENTRY* data() const override
        {
            static const only_true dummy = _Loaded(true);
            (void)dummy;
            return _Data();
        }

        bool loaded() const override
        {
            return _Loaded(false);
        }

        template <chars_cache Interface>
        basic_address<void> find_interface() const
        {
            static const auto found = fd::find_csgo_interface(this->find_export<"CreateInterface">(), Interface, this->data());
            return found;
        }
    };

    struct current_module final : any_module<0>
    {
        LDR_DATA_TABLE_ENTRY* data() const override;
        bool loaded() const override;
    };

#define DLL_NAME(_NAME_)    L"" #_NAME_ ".dll"
#define GAME_MODULE(_NAME_) constexpr rt_module<DLL_NAME(_NAME_)> _NAME_;

    export namespace rt_modules
    {
        constexpr current_module current;

        FOR_EACH(GAME_MODULE,
                 server,
                 client,
                 engine,
                 datacache,
                 materialsystem,
                 vstdlib,
                 vgui2,
                 vguimatsurface,
                 vphysics,
                 inputsystem,
                 studiorender,
                 shaderapidx9,
                 d3d9,
                 serverbrowser);

    } // namespace rt_modules

} // namespace fd
