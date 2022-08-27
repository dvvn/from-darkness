module;

#include <fd/assert.h>

#include <variant>

module fd.netvars.info;
import fd.netvars.type_resolve;

using namespace fd;
using namespace valve;

static const char* _Netvar_name(const netvar_info_source source)
{
    return std::visit(
        [](auto src) {
            return src->name;
        },
        source);
}

static string _Netvar_type(const netvar_info_source source)
{
    struct
    {
        string operator()(const recv_prop* rp) const
        {
            return netvars::type_recv_prop(rp);
        }

        string operator()(const data_map_description* td) const
        {
            return netvars::type_datamap_field(td);
        }
    } type_getter;

    return std::visit(type_getter, source);
}

//---

netvar_info::netvar_info(const size_t offset, const netvar_info_source source, const size_t size, const hashed_string_view name)
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

hashed_string_view netvar_info::name() const
{
    if (name_.empty())
    {
        string_view name = _Netvar_name(source_);
        if (size_ > 0)
        {
            FD_ASSERT(name.ends_with("[0]"));
            name.remove_suffix(3);
            FD_ASSERT(name.rfind(']') == name.npos);
        }
        name_ = name;
    }
    return name_;
}

string_view netvar_info::type() const
{
    if (type_.empty())
    {
        auto tmp_type = _Netvar_type(source_);

        if (size_ <= 1)
        {
            type_ = std::move(tmp_type);
        }
        else
        {
            string_view netvar_type;
            if (size_ == 3)
            {
                netvar_type = std::visit(
                    [&](auto ptr) {
                        return netvars::type_array_prefix(tmp_type, ptr);
                    },
                    source_);
            }

            if (netvar_type.empty())
                type_ = netvars::type_std_array(tmp_type, size_);
            else
                type_ = netvar_type;
        }
    }
    return type_;
}

//----

netvar_info_instant::netvar_info_instant(const size_t offset, const hashed_string_view name, string&& type)
    : offset_(offset)
    , name_(name)
    , type_(std::move(type))
{
}

size_t netvar_info_instant::offset() const
{
    return offset_;
}

hashed_string_view netvar_info_instant::name() const
{
    return name_;
}

string_view netvar_info_instant::type() const
{
    return type_;
}
