#include <fd/assert.h>
#include <fd/netvar_info.h>
#include <fd/netvar_type_resolve.h>

namespace fd
{
netvar_info::netvar_info(const size_t offset, const netvar_info_source source, const size_t size, const string_view name)
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

string_view netvar_info::name() const
{
    if (name_.empty())
    {
        auto name = std::visit(
            [](auto src) -> string_view {
                return src->name;
            },
            source_
        );
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
        constexpr auto typeGetter = overload(extract_type, extract_type);

        std::visit(
            [&](auto val) {
                auto tmpType = typeGetter(val);
                // ReSharper disable once CppUnreachableCode
                if (size_ <= 1)
                {
                    type_ = std::move(tmpType);
                    return;
                }
                if (size_ == 3)
                {
                    type_ = type_array_prefix(tmpType, val);
                    if (!type_.empty())
                        return;
                }

                type_ = extract_type_std_array(tmpType, size_);
            },
            source_
        );
    }
    return type_;
}

//----

netvar_info_instant::netvar_info_instant(const size_t offset, const string_view name, string&& type)
    : offset_(offset)
    , name_(name)
    , type_(std::move(type))
{
}

size_t netvar_info_instant::offset() const
{
    return offset_;
}

string_view netvar_info_instant::name() const
{
    return name_;
}

string_view netvar_info_instant::type() const
{
    return type_;
}
}