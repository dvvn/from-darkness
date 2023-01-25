#include <fd/algorithm.h>
#include <fd/assert.h>
#include <fd/netvar_table.h>
#include <fd/views.h>

namespace fd
{

netvar_table::~netvar_table()
{
    for (const auto ptr : reverse_view(storage_))
        delete ptr;
}

netvar_table::netvar_table(string&& name, const bool root)
    : name_(std::move(name))
    , isRoot_(root)
{
    FD_ASSERT(!name_.empty(), "Incorrect name");
}

netvar_table::netvar_table(netvar_table&& other) noexcept = default;

string_view netvar_table::name() const
{
    return name_;
}

bool netvar_table::root() const
{
    return isRoot_;
}

const basic_netvar_info* netvar_table::find(const string_view name) const
{
    FD_ASSERT(!name.empty());

    for (const auto entry : storage_)
    {
        if (entry->name() == name)
            return entry;
    }
    return nullptr;
}

void netvar_table::add(basic_netvar_info* info)
{
#ifdef _DEBUG
    if (!storage_.empty())
    {
        const auto name   = info->name();
        const auto offset = info->offset();
        const auto type   = info->type();

        for (auto item : storage_)
        {
            if (item->name() == name)
                FD_ASSERT("Item with given name already added!");

            if (item->offset() == offset)
            {
                const auto currType = item->type();
                if (currType.empty() || /*type.empty() ||*/ type == currType)
                    FD_ASSERT("Item with given offset and type already added!");
                // othervise skip this offset manually
            }
        }
    }
#endif

    storage_.emplace_back(info);
}

void netvar_table::sort()
{
    std::sort(storage_.begin(), storage_.end(), [](auto l, auto r) {
        return l->offset() < r->offset();
    });
}

bool netvar_table::empty() const
{
    return storage_.empty();
}

basic_netvar_info** netvar_table::begin()
{
    return storage_.data();
}

basic_netvar_info** netvar_table::end()
{
    return &storage_.back() + 1;
}

//----

/* bool netvar_table_multi::have_inner() const
{
    return std::holds_alternative<netvar_table_multi>(inner_);
}

netvar_table_multi& netvar_table_multi::inner(string&& name)
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