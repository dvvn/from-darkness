#include <fd/library_info/dos.h>
#include <fd/library_info/export.h>

#include <windows.h>
#include <winternl.h>

#include <cassert>

namespace fd
{
class dll_exports
{
    dos_header dos_;

    uint32_t *names_;
    uint32_t *funcs_;
    uint16_t *ords_;

    union
    {
        IMAGE_EXPORT_DIRECTORY *exportDirDir_;
        uint8_t *virtualAddrStart_;
    };

    uint8_t *virtualAddrEnd_;

    using src_ptr = dll_exports const *;

  public:
    dll_exports(IMAGE_DOS_HEADER *dos, IMAGE_NT_HEADERS *nt)
        : dos_(dos)
    {
        auto const &dataDir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];

        // get export export_dir.
        exportDirDir_   = dos_ + dataDir.VirtualAddress;
        virtualAddrEnd_ = virtualAddrStart_ + dataDir.Size;

        names_ = dos_ + exportDirDir_->AddressOfNames;
        funcs_ = dos_ + exportDirDir_->AddressOfFunctions;
        ords_  = dos_ + exportDirDir_->AddressOfNameOrdinals;
    }

    char const *name(DWORD offset) const
    {
        return dos_ + names_[offset];
    }

    void *function(DWORD offset) const
    {
        void *tmp = dos_ + funcs_[ords_[offset]];
        if (tmp < virtualAddrStart_ || tmp >= virtualAddrEnd_)
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
            return { src_, offset_ };
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
        return { this, 0 };
    }

    iterator end() const
    {
        return { this, std::min(exportDirDir_->NumberOfNames, exportDirDir_->NumberOfFunctions) };
    }
};

void *_find_export(IMAGE_DOS_HEADER *dos, IMAGE_NT_HEADERS *nt, const char *name, size_t length)
{
    for (auto val : dll_exports(dos, nt))
    {
        if (memcmp(val.name(), name, length) == 0)
            return val.function();
    }
    return nullptr;
}
}