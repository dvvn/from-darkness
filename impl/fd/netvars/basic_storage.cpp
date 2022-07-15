module;

#include <fd/assert.h>

#include <functional>
#include <ranges>
#include <variant>

module fd.netvars.core:basic_storage;
import :type_resolve;
import fd.string;

using namespace fd;
using namespace valve;
using namespace netvars;

static const char* _Netvar_name(const netvar_info_source source)
{
    return std::visit(
        []<class T>(const T* src) {
            return std::invoke(&T::name, src);
        },
        source);
}

static string_or_view _Netvar_type(const netvar_info_source source)
{
    struct netvar_type_getter
    {
        string_or_view operator()(const recv_prop* const rp) const
        {
            return type_recv_prop(rp);
        }

        string_or_view operator()(const data_map_description* const td) const
        {
            return type_datamap_field(td);
        }
    };

    return std::visit(netvar_type_getter(), source);
}

//---

netvar_info::netvar_info(const size_t offset, const netvar_info_source source, const size_t size, const fd::hashed_string_view name)
    : offset_(offset)
    , source_(source)
    , size_(size)
    , name_(name)
{
}

size_t netvar_info::offset() const
{
    return offset_;
}

// const void* netvar_info::source( ) const
//{
//	const auto addr = reinterpret_cast<uintptr_t>(std::addressof(type_));
//	const auto inner_ptr = reinterpret_cast<void**>(addr);
//	return *inner_ptr;
// }

fd::hashed_string_view netvar_info::name() const
{
    if (name_.empty())
    {
        if (size_ == 0)
        {
            name_ = _Netvar_name(source_);
        }
        else
        {
            fd::string_view name = _Netvar_name(source_);
            FD_ASSERT(name.ends_with("[0]"));
            name.remove_suffix(3);
            FD_ASSERT(name.rfind(']') == name.npos);
            name_ = name;
        }
    }
    return name_;
}

fd::string_view netvar_info::type() const
{
    const fd::string_view type_strv = type_;
    if (!type_strv.empty())
        return type_strv;

    auto type = _Netvar_type(source_);

    if (size_ <= 1)
    {
        type_ = std::move(type);
    }
    else
    {
        fd::string_view netvar_type;
        if (size_ == 3)
        {
            netvar_type = std::visit(
                [&]<class T>(T* const ptr) {
                    return type_array_prefix(type, ptr);
                },
                source_);
        }

        if (netvar_type.empty())
            type_ = type_std_array(type, size_);
        else
            type_ = netvar_type;
    }

    return type_;
}

//----

netvar_info_custom_constant::netvar_info_custom_constant(const size_t offset, const fd::hashed_string_view name, string_or_view&& type)
    : offset_(offset)
    , name_(name)
    , type_(type)
{
}

size_t netvar_info_custom_constant::offset() const
{
    return offset_;
}

fd::hashed_string_view netvar_info_custom_constant::name() const
{
    return name_;
}

fd::string_view netvar_info_custom_constant::type() const
{
    return type_;
}

//----

void netvar_table::validate_item(const basic_netvar_info* info) const
{
#ifdef _DEBUG
    const auto name = info->name();
    FD_ASSERT(!name.empty(), "Item name not set!");

    if (this->empty())
        return;

    const auto offset = info->offset();
    const auto type   = info->type();

    for (auto& item : *this)
    {
        if (item->name() == name)
            FD_ASSERT("Item with given name already added!");

        if (item->offset() == offset)
        {
            const auto type_curr = item->type();
            if (type_curr.empty() || type.empty() || type == type_curr)
                FD_ASSERT("Item with given offset and type already added!");
            // othervise skip this offset manually
        }
    }
#else
    (void)info;
    (void)this;
#endif
}

netvar_table::netvar_table(fd::hashed_string&& name)
    : name_(std::move(name))
{
}

fd::hashed_string_view netvar_table::name() const
{
    return name_;
}

const basic_netvar_info* netvar_table::find(const fd::hashed_string_view name) const
{
    for (auto& entry : *this)
    {
        if (entry->name() == name)
            return entry.get();
    }
    return nullptr;
}

const netvar_info* netvar_table::add(const size_t offset, const netvar_info_source source, const size_t size, const fd::hashed_string_view name)
{
    return add_impl<netvar_info>(offset, source, size, name);
}

const netvar_info_custom_constant* netvar_table::add(const size_t offset, const fd::hashed_string_view name, string_or_view&& type)
{
    return add_impl<netvar_info_custom_constant>(offset, name, std::move(type));
}

//----

// bool asic_storage::contains_duplicate(const fd::hashed_string_view name, netvar_table* const from) const
//{
//	const auto begin = this->data( );
//	const auto end = begin + this->size( );
//
//	const auto pos = from ? from : begin;
//	FD_ASSERT(std::distance(begin, pos) >= 0);
//
//	const auto found1 = _Find_name(std::span(pos, end), name);
//	if(found1)
//		return _Find_name(std::span(found1 + 1, end), name);
//	return false;
// }

auto basic_storage::find(const fd::hashed_string_view name) const -> const netvar_table*
{
    return const_cast<basic_storage*>(this)->find(name);
}

auto basic_storage::find(const fd::hashed_string_view name) -> netvar_table*
{
    for (netvar_table& entry : *this)
    {
        if (entry.name() != name)
            continue;
        return std::addressof(entry);
    }
    return nullptr;
}

auto basic_storage::add(netvar_table&& table, const bool skip_find) -> netvar_table*
{
    if (!skip_find)
    {
        const auto existing = find(table.name());
        if (existing)
            return existing;
    }
    this->push_back(std::move(table));
    return nullptr;
}

auto basic_storage::add(fd::hashed_string&& name, const bool skip_find) -> netvar_table*
{
    if (!skip_find)
    {
        const auto existing = find(name);
        if (existing)
            return existing;
    }
    this->emplace_back(std::move(name));
    return nullptr;
}
