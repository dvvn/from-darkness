#include <fd/netvars/table.h>

#include <spdlog/spdlog.h>

namespace fd
{
static basic_netvar_info* new_clone(basic_netvar_info const& other)
{
    return other.clone();
}

netvar_table::~netvar_table() = default;

netvar_table::netvar_table(std::string&& name, bool const root)
    : name_(std::move(name))
    , isRoot_(root)
{
    assert(!name_.empty());
}

netvar_table::netvar_table(netvar_table&& other) noexcept = default;

std::string_view netvar_table::name() const
{
    return name_;
}

bool netvar_table::root() const
{
    return isRoot_;
}

basic_netvar_info const* netvar_table::find(const std::string_view name) const
{
    assert(!name.empty());
    for (auto const& entry : storage_)
    {
        if (entry.name() == name)
            return &entry;
    }
    return nullptr;
}

struct _unique_name
{
    netvar_table*      table;
    basic_netvar_info* info;
};

void netvar_table::add(basic_netvar_info* info)
{
    assert(!find(info->name()));
    storage_.push_back((info));
}

static bool operator<(basic_netvar_info const& l, basic_netvar_info const& r)
{
    return l.offset() < r.offset();
}

void netvar_table::sort()
{
    std::stable_sort(
        storage_.begin().base(),
        storage_.end().base(),
        boost::make_void_ptr_indirect_fun<basic_netvar_info>(std::less())
    );
    // storage_.sort();
}

auto netvar_table::begin() -> iterator
{
    return storage_.begin();
}

auto netvar_table::end() -> iterator
{
    return storage_.end();
}

auto netvar_table::begin() const -> const_iterator
{
    return storage_.begin();
}

auto netvar_table::end() const -> const_iterator
{
    return storage_.end();
}

bool netvar_table::empty() const
{
    return storage_.empty();
}

size_t netvar_table::size() const
{
    return storage_.size();
}

//----

/* bool netvar_table_multi::have_inner() const
{
    return std::holds_alternative<netvar_table_multi>(inner_);
}

netvar_table_multi& netvar_table_multi::inner(std::string&& name)
{
    FD_ASSERT(!have_inner());
    auto& inner = inner_.emplace<netvar_table_multi>();
    inner.construct(std::move(name));
    return inner;
}

netvar_table_multi& netvar_table_multi::inner()
{
    return std::get<netvar_table_multi>(inner_);
}

const netvar_table_multi& netvar_table_multi::inner() const
{
    return std::get<netvar_table_multi>(inner_);
} */
}