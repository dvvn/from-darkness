#include "export.h"

#include <fd/tool/functional.h>
#include <fd/tool/string_view.h>

#include <windows.h>
#include <winternl.h>

#include <algorithm>
#include <cassert>

namespace fd
{
using placeholders::_1;

class cast_helper
{
    void *from_;

  public:
    cast_helper(void *from)
        : from_(from)
    {
    }

    template <typename Q>
    operator Q() const
    {
        return static_cast<Q>(from_);
    }
};

struct export_data
{
    union
    {
        IMAGE_DOS_HEADER *dos_header;
        uint8_t *dos;
    };

    union
    {
        IMAGE_EXPORT_DIRECTORY *export_dir;
        uint8_t *virtual_addr_start;
    };

    uint8_t *virtual_addr_end;

    uint32_t *names;
    uint32_t *funcs;
    uint16_t *ords;

    export_data(IMAGE_DOS_HEADER *dos_header, IMAGE_NT_HEADERS *nt_header)
        : dos_header(dos_header)
    {
        auto &data_dir = nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];

        virtual_addr_start = cast_helper(dos + data_dir.VirtualAddress);
        virtual_addr_end   = cast_helper(virtual_addr_start + data_dir.Size);

        names = cast_helper(dos + export_dir->AddressOfNames);
        funcs = cast_helper(dos + export_dir->AddressOfFunctions);
        ords  = cast_helper(dos + export_dir->AddressOfNameOrdinals);
    }
};

class export_view
{
    size_t offset_;
    export_data *data_;

  public:
    export_view(size_t offset, export_data *data)
        : offset_(offset)
        , data_(data)
    {
    }

    char const *name() const
    {
        return reinterpret_cast<char const *>(data_->dos + data_->names[offset_]);
    }

    void *function() const
    {
        void *tmp = data_->dos + data_->funcs[data_->ords[offset_]];
        if (tmp < data_->virtual_addr_start || tmp >= data_->virtual_addr_end)
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
};

template <std::invocable<char const *> Filter>
static void *find_export(IMAGE_DOS_HEADER *dos, IMAGE_NT_HEADERS *nt, Filter filter)
{
    auto edata = export_data(dos, nt);

    size_t last_offset = std::min(edata.export_dir->NumberOfNames, edata.export_dir->NumberOfFunctions);
    for (size_t offset = 0; offset != last_offset; ++offset)
    {
        auto view = export_view(offset, &edata);
        if (filter(view.name()))
            return view.function();
    }
    return nullptr;
}

void *find_export(IMAGE_DOS_HEADER *dos, IMAGE_NT_HEADERS *nt, char const *name, size_t length)
{
    return find_export(dos, nt, bind(memcmp, _1, name, length) == 0);
}

void *find_export(IMAGE_DOS_HEADER *dos, IMAGE_NT_HEADERS *nt, string_view name)
{
    return find_export(dos, nt, _1 == name);
}
}