#include "hash.h"
#include "info.h"

#include <cassert>

namespace fd
{
#if 0
netvar_info::netvar_info(size_t offset, netvar_source source, std::string_view name, size_t arraySize)
    : offset_(offset)
    , source_(source)
    , arraySize_(arraySize)
    , name_(name)
{
    assert(!std::holds_alternative<std::monostate>(source_));
    assert(!name_.empty());

    if (std::holds_alternative<valve::recv_prop*>(source_) &&
        std::get<valve::recv_prop*>(source_)->type == valve::DPT_Array)
    {
        assert(arraySize == 0); // size resolved automativally
        if (name_.starts_with('\"'))
            name_.remove_prefix(1);
        if (name_.ends_with('\"'))
            name_.remove_suffix(1);
    }
}

netvar_info::netvar_info(size_t offset, std::string_view name, std::string_view type, size_t arraySize)
    : offset_(offset)
    , arraySize_(arraySize)
    , name_(name)
    , type_(arraySize == 0 ? type : extract_type_std_array(type, arraySize))
{
    assert(!name.empty());
    assert(!type.empty());
}
#endif

hashed_netvar_name::hashed_netvar_name(std::string_view name)
    : std::pair<size_t, std::string_view>(netvar_hash(name), name)
{
    assert(!name.empty());
}

hashed_netvar_name::hashed_netvar_name(const char *name)
    : hashed_netvar_name(std::string_view(name))
{
}

bool hashed_netvar_name::operator==(hashed_netvar_name const &other) const
{
    return std::get<size_t>(*this) == std::get<size_t>(other);
}

bool hashed_netvar_name::operator==(std::string_view other) const
{
    return std::get<std::string_view>(*this) == other;
}

std::string_view const *hashed_netvar_name::operator->() const
{
    return &std::get<std::string_view>(*this);
}

std::string_view hashed_netvar_name::get() const
{
    return std::get<std::string_view>(*this);
}

netvar_info::netvar_info(size_t offset, uint16_t array_size, netvar_source source, hashed_netvar_name const &name)
    : offset_(offset)
    , array_size_(array_size)
    , source_(source)
    , name_((name))

{
}

netvar_info::netvar_info(size_t offset, netvar_source source, hashed_netvar_name const &name)
    : offset_(offset)
    , array_size_(0)
    , source_(source)
    , name_((name))

{
}

size_t netvar_info::offset() const
{
    return offset_;
}

// const void* netvar_info::source( ) const
//{
//	auto addr = reinterpret_cast<uintptr_t>(std::addressof(type_));
//	auto inner_ptr = reinterpret_cast<void**>(addr);
//	return *inner_ptr;
// }

std::string_view netvar_info::name() const
{
#if 0
    if (name_.empty())
    {
        std::string_view name = std::visit(
            []<typename T>(T src) -> char const*
            {
                if constexpr (std::same_as<std::monostate, T>)
                    std::terminate();
                else
                    return src->name;
            },
            source_);
        if (arraySize_ > 0)
        {
            assert(name.starts_with("m_"));
            name.remove_suffix(3);
        }
        name_ = name;
    }
#endif
    return std::get<std::string_view>(name_);
}

std::string_view netvar_info::type() const
{
    return source_.type(name(), array_size_)->get();
}

size_t netvar_info::array_size() const
{
    /*if (std::holds_alternative<netvar_type_array>(type_.data))
        return std::get<netvar_type_array>(type_.data).size;*/
    return netvar_type_array_size(source_.type(name(), array_size_));
}

bool netvar_info::operator==(netvar_info const &other) const
{
    return name_ == other.name_ &&             //
           offset_ == other.offset_ &&         //
           array_size_ == other.array_size_ && //
           source_ == other.source_;
}

bool netvar_info::operator==(std::string_view name) const
{
    return std::get<std::string_view>(name_) == name;
}

bool netvar_info::operator==(hashed_netvar_name const &name_hash) const
{
    return name_ == name_hash;
}

bool netvar_info::operator==(size_t name_hash) const
{
    return std::get<size_t>(name_) == name_hash;
}
}