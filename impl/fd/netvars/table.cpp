module;

#include <fd/assert.h>

#include <ranges>
#include <variant>

module fd.netvars.table;

using namespace fd;
using namespace valve;

void netvar_table::validate_item(const basic_netvar_info* info) const
{
    if (this->empty())
        return;

    const auto name   = info->name();
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
}

netvar_table::netvar_table(hashed_string&& name, const bool root)
    : name_(std::move(name))
    , root_(root)
{
    FD_ASSERT(!name_.empty(), "Incorrect name");
}

hashed_string_view netvar_table::name() const
{
    return name_;
}

bool netvar_table::root() const
{
    return root_;
}

const basic_netvar_info* netvar_table::find(const hashed_string_view name) const
{
    FD_ASSERT(!name.empty());

    for (auto& entry : *this)
    {
        if (entry->name() == name)
            return entry.get();
    }
    return nullptr;
}

//----

/* bool netvar_table_multi::have_inner() const
{
    return std::holds_alternative<netvar_table_multi>(inner_);
}

netvar_table_multi& netvar_table_multi::inner(hashed_string&& name)
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
