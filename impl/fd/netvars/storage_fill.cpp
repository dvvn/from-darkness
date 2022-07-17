module;

#include <fd/assert.h>

//#include <format>
#include <algorithm>
#include <ranges>
#include <variant>

module fd.netvars.core:storage;
import fd.ctype;

using namespace fd;
using namespace netvars;

static auto _Correct_class_name(const fd::string_view name)
{
    fd::string ret;

    if (name[0] == 'C' && name[1] != '_')
    {
        FD_ASSERT(fd::is_alnum(name[1]));
        // internal csgo classes looks like C_***
        // same classes in shared code look like C***
        ret = fd::make_string("C_", name.substr(1));
    }
    else
    {
        FD_ASSERT(!name.starts_with("DT_"));
        ret.assign(name);
    }

    return ret;
}

static bool _Can_skip_netvar(const char* name)
{
    for (;;)
    {
        const auto c = *++name;
        if (c == '.')
            return true;
        if (c == '\0')
            return false;
    }
}

#ifndef __cpp_lib_string_contains
#define contains(_X_) find(_X_) != static_cast<size_t>(-1)
#endif

static bool _Can_skip_netvar(const fd::string_view name)
{
    return name.contains('.');
}

using namespace fd::valve;

static bool _Table_is_array(const recv_table& table)
{
    return /*!table.props.empty( ) &&*/ fd::is_digit(table.props.back().name[0]);
};

static bool _Table_is_data_table(const recv_table& table)
{
    // DT_*****
    return std::memcmp(table.name, "DT_", 3) == 0 && table.name[3] != '\0';
};

static auto _Get_props_range(const recv_table* recv_table)
{
    constexpr auto is_base_class = [](const recv_prop* prop) {
        constexpr fd::string_view str = "baseclass";
        return std::memcmp(prop->name, str.data(), str.size()) == 0 && prop->name[str.size()] == '\0';
    };

    constexpr auto is_length_proxy = [](const recv_prop* prop) {
        if (prop->array_length_proxy)
            return true;

        const auto lstr = fd::to_lower(prop->name);
        return lstr.contains("length") && lstr.contains("proxy");
    };

    const auto& raw_props = recv_table->props;

    recv_prop* front = raw_props.data();
    recv_prop* back  = front + raw_props.size() - 1;

    if (is_base_class(front))
        ++front;
    if (is_length_proxy(back))
        --back;

    return std::make_pair(front, back + 1);
}

struct recv_prop_array_info
{
    fd::string_view name;
    size_t size = 0;
};

// other_props = {itr+1, end}
static recv_prop_array_info _Parse_prop_array(const fd::string_view first_prop_name, const std::span<const recv_prop> other_props, const netvar_table& tree)
{
    if (!first_prop_name.ends_with("[0]"))
        return {};

    const fd::string_view real_prop_name = first_prop_name.substr(0, first_prop_name.size() - 3);
    FD_ASSERT(!real_prop_name.ends_with(']'));
    if (tree.find(real_prop_name)) // todo: debug break for type check!
        return { real_prop_name, 0 };

    // todo: try extract size from length proxy
    size_t array_size = 1;

    for (const auto& prop : other_props)
    {
        if (prop.type != prop.type) // todo: check is name still same after this (because previously we store this name without array braces)
            break;

        // name.starts_with(real_prop_name)
        if (std::memcmp(prop.name, real_prop_name.data(), real_prop_name.size()) != 0)
            break;

        // name.size() == real_prop_name.size()
        if (prop.name[real_prop_name.size()] != '\0')
            break;

        ++array_size;
    }

    return { real_prop_name, array_size };
}

static void _Parse_client_class(storage& root_tree, netvar_table& tree, recv_table* const recv_table, const size_t offset)
{
    const auto [props_begin, props_end] = _Get_props_range(recv_table);

    for (auto itr = props_begin; itr != props_end; ++itr)
    {
        const auto& prop = *itr;
        FD_ASSERT(prop.name != nullptr);
        const fd::string_view prop_name = prop.name;
        if (_Can_skip_netvar(prop_name))
            continue;

        const auto real_prop_offset = offset + prop.offset;

        if (prop_name.rfind(']') != prop_name.npos)
        {
            const auto array_info = _Parse_prop_array(prop_name, { itr + 1, props_end }, tree);
            if (array_info.size > 0)
            {
                tree.add(real_prop_offset, itr, array_info.size, array_info.name);
                itr += array_info.size - 1;
            }
        }
        else if (prop.type != DPT_DataTable)
        {
            tree.add(real_prop_offset, itr, 0, prop_name);
        }
        else if (prop.data_table && !prop.data_table->props.empty())
        {
            _Parse_client_class(root_tree, tree, prop.data_table, real_prop_offset);
        }
    }
}

void storage::iterate_client_class(client_class* root_class)
{
    for (auto client_class = root_class; client_class != nullptr; client_class = client_class->next)
    {
        const auto rtable = client_class->table;
        if (!rtable || rtable->props.empty())
            continue;

        hashed_string class_name = _Correct_class_name(client_class->name);
        FD_ASSERT(this->find(class_name));
        const auto added = this->add(std::move(class_name), true);

        _Parse_client_class(*this, *added, rtable, 0);

        /*if (added->empty( ))
            this->erase(added.data( ));*/
    }
}

static void _Parse_datamap(netvar_table& tree, data_map* const map)
{
    /* for (auto& desc : map->data)
    {
        if (desc.type == FIELD_EMBEDDED)
        {
            if (desc.TypeDescription != nullptr)
                FD_ASSERT("Embedded datamap detected");
        }
        else if (desc.fieldName != nullptr)
        {
            const fd::string_view name = desc.fieldName;
            if (_Can_skip_netvar(name))
                continue;
            tree.add(static_cast<size_t>(desc.fieldOffset[TD_OFFSET_NORMAL]), std::addressof(desc), 0, name);
        }
    } */
}

void storage::iterate_datamap(data_map* const root_map)
{
    for (auto map = root_map; map != nullptr; map = map->base)
    {
        if (map->data.empty())
            continue;

        hashed_string class_name = _Correct_class_name(map->name);
        const auto added         = this->add(std::move(class_name));

        _Parse_datamap(*added, map);

        /*if (added->empty( ))
            this->erase(added.data( ));*/
    }
}
