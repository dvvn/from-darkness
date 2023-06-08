#include "export.h"

#include <windows.h>
#include <winternl.h>

#include <algorithm>
#include <cassert>

template <typename To, typename From>
static FORCEINLINE void ptr_cast(To *&to, From *from)
{
    to = reinterpret_cast<To *>(from);
}

namespace fd
{
class dll_exports
{
    uint8_t *dos_;

    uint32_t *names_;
    uint32_t *funcs_;
    uint16_t *ords_;

    union
    {
        IMAGE_EXPORT_DIRECTORY *export_dir_;
        uint8_t *virtual_addr_start_;
    };

    uint8_t *virtual_addr_end_;

    using src_ptr = dll_exports const *;

  public:
    dll_exports(IMAGE_DOS_HEADER *dos, IMAGE_NT_HEADERS *nt)
    {
        ptr_cast(dos_, dos);
        auto const &data_dir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];

        // get export export_dir.
        ptr_cast(export_dir_, dos_ + data_dir.VirtualAddress);
        ptr_cast(virtual_addr_end_, virtual_addr_start_ + data_dir.Size);

        ptr_cast(names_, dos_ + export_dir_->AddressOfNames);
        ptr_cast(funcs_, dos_ + export_dir_->AddressOfFunctions);
        ptr_cast(ords_, dos_ + export_dir_->AddressOfNameOrdinals);
    }

    char const *name(DWORD offset) const
    {
        return reinterpret_cast<char const *>(dos_ + names_[offset]);
    }

    void *function(DWORD offset) const
    {
        void *tmp = dos_ + funcs_[ords_[offset]];
        if (tmp < virtual_addr_start_ || tmp >= virtual_addr_end_)
            return tmp;

        // todo:resolve fwd export
        assert(0 && "Forwarded export detected");
        return nullptr;

#if 0
		// get forwarder std::string.
		std::string_view fwd_str = export_ptr.get<const char*>( );

		// forwarders have a period as the delimiter.
		auto delim = fwd_str.find_last_of('.');
		if(delim == fwd_str.npos)
			continue;

		using namespace string_view_literals;
		// get forwarder mod name.
		const info_string::fixed_type fwd_module_str = nstd::append<std::wstring>(fwd_str.substr(0, delim), L".dll"sv);

		// get real export ptr ( recursively ).
		auto target_module = std::ranges::find_if(*all_modules, [&](const info& i)
		{
			return i.name == fwd_module_str;
		});
		if(target_module == all_modules->end( ))
			continue;

		// get forwarder export name.
		auto fwd_export_str = fwd_str.substr(delim + 1);

		try
		{
			auto& exports = target_module->exports( );
			auto fwd_export_ptr = exports.at(fwd_export_str);

			this->emplace(export_name, fwd_export_ptr);
		}
		catch(std::exception)
		{
		}
#endif
    }

    class wrapper
    {
        src_ptr src_;
        DWORD offset_;

      public:
        wrapper(src_ptr src, DWORD offset)
            : src_(src)
            , offset_(offset)
        {
        }

        char const *name() const
        {
            return src_->name(offset_);
        }

        void *function() const
        {
            return src_->function(offset_);
        }
    };

    class iterator
    {
        src_ptr src_;
        DWORD offset_;

      public:
        iterator(src_ptr src, DWORD offset)
            : src_(src)
            , offset_(offset)
        {
        }

        iterator &operator++()
        {
            ++offset_;
            return *this;
        }

        iterator &operator--()
        {
            --offset_;
            return *this;
        }

        wrapper operator*() const
        {
            return {src_, offset_};
        }

        bool operator==(iterator const &other) const
        {
            if (offset_ == other.offset_)
            {
                assert(this->src_ == other.src_);
                return true;
            }
            return false;
        }
    };

    iterator begin() const
    {
        return {this, 0};
    }

    iterator end() const
    {
        return {this, std::min(export_dir_->NumberOfNames, export_dir_->NumberOfFunctions)};
    }
};

void *find_export(IMAGE_DOS_HEADER *dos, IMAGE_NT_HEADERS *nt, char const *name, size_t length)
{
    for (auto val : dll_exports(dos, nt))
    {
        if (memcmp(val.name(), name, length) == 0)
            return val.function();
    }
    return nullptr;
}
}