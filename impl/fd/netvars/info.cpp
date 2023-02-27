#include <fd/netvars/info.h>
#include <fd/netvars/type_resolve.h>

#include <cassert>

namespace fd
{

netvar_info::netvar_info(size_t offset, netvar_info_source source, std::string_view name, size_t arraySize)
    : offset_(offset)
    , source_(source)
    , arraySize_(arraySize)
    , name_(name)
{
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
    if (name_.empty())
    {
        std::string_view name = std::visit([](auto* src) -> char const* { return src->name; }, source_);
        if (arraySize_ > 0)
        {
            assert(name.starts_with("m_"));
            name.remove_suffix(3);
        }
        name_ = name;
    }
    return name_;
}

std::string_view netvar_info::type() const
{
    if (type_.empty())
    {
        std::visit(
            [&](auto val)
            {
                auto name = this->name();
                if (arraySize_ <= 1)
                {
                    type_ = extract_type(name, val);
                    return;
                }
                if (arraySize_ == 3)
                {
                    type_ = extract_type_by_prefix(name, val);
                    if (!type_.empty())
                        return;
                }
                type_ = extract_type_std_array(extract_type(name, val), arraySize_);
            },
            source_);
    }
    return type_;
}

size_t netvar_info::array_size() const
{
    return arraySize_;
}

//----

netvar_info_instant::netvar_info_instant(size_t offset, std::string_view name, std::string type)
    : offset_(offset)
    , name_(name)
    , type_(std::move(type))
{
    assert(!type_.empty());
}

netvar_info_instant::netvar_info_instant(size_t offset, std::string_view name, std::string_view type, size_t arraySize)
    : offset_(offset)
    , name_(name)
    , type_(extract_type_std_array(type, arraySize))
{
    assert(!type.empty());
    assert(arraySize != 0);
}

size_t netvar_info_instant::offset() const
{
    return offset_;
}

std::string_view netvar_info_instant::name() const
{
    return name_;
}

std::string_view netvar_info_instant::type() const
{
    return type_;
}
} // namespace fd