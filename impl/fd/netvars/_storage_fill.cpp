module;

#include <fd/assert.h>

//#include <format>
#include <algorithm>
#include <ranges>

module fd.netvars.core:storage;
import fd.ctype;
import fd.string.make;

using namespace fd;
using namespace netvars;

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

static void _Parse_client_class(storage& root_tree, netvar_table& tree, recv_table* const recv_table, const size_t offset)
{
}

void storage::iterate_client_class(client_class* root_class)
{
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
