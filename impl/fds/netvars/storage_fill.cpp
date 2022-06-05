module;

#include <fds/core/assert.h>
#include <nstd/format.h>
#include <nstd/ranges.h>

#include <algorithm>
#include <cctype>
#include <variant>

module fds.netvars.core:storage;
import nstd.text.convert;

using namespace fds;
using namespace netvars;
using namespace csgo;
using namespace nstd::text;

static auto _Correct_class_name(const std::string_view name)
{
    std::string ret;

    if (name[0] == 'C' && name[1] != '_')
    {
        fds_assert(std::isalnum(name[1]));
        // internal csgo classes looks like C_***
        // same classes in shared code look like C***
        ret.reserve(2 + name.size() - 1);
        ret += "C_";
        ret += name.substr(1);
    }
    else
    {
        fds_assert(!name.starts_with("DT_"));
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

static bool _Can_skip_netvar(const std::string_view name)
{
    return name.contains('.');
}

static bool _Table_is_array(const RecvTable& table)
{
    return /*!table.props.empty( ) &&*/ std::isdigit(table.props.back().m_pVarName[0]);
};

static bool _Table_is_data_table(const RecvTable& table)
{
    // DT_*****
    return std::memcmp(table.m_pNetTableName, "DT_", 3) == 0 && table.m_pNetTableName[3] != '\0';
};

static auto _Get_props_range(const RecvTable* recv_table)
{
    constexpr auto is_base_class = [](const RecvProp* prop) {
        constexpr std::string_view str = "baseclass";
        return std::memcmp(prop->m_pVarName, str.data(), str.size()) == 0 && prop->m_pVarName[str.size()] == '\0';
    };

    constexpr auto is_length_proxy = [](const RecvProp* prop) {
        if (prop->m_ArrayLengthProxy)
            return true;

        const auto lstr = nstd::text::to_lower(prop->m_pVarName);
        return lstr.contains("length") && lstr.contains("proxy");
    };

    const auto& raw_props = recv_table->props;

    RecvProp* front = raw_props.data();
    RecvProp* back  = front + raw_props.size() - 1;

    if (is_base_class(front))
        ++front;
    if (is_length_proxy(back))
        --back;

    return std::make_pair(front, back + 1);
}

struct recv_prop_array_info
{
    std::string_view name;
    size_t size = 0;
};

// other_props = {itr+1, end}
static recv_prop_array_info _Parse_prop_array(const std::string_view first_prop_name, const std::span<const RecvProp> other_props, const netvar_table& tree)
{
    if (!first_prop_name.ends_with("[0]"))
        return {};

    const std::string_view real_prop_name = first_prop_name.substr(0, first_prop_name.size() - 3);
    fds_assert(!real_prop_name.ends_with(']'));
    if (tree.find(real_prop_name)) // todo: debug break for type check!
        return {real_prop_name, 0};

    // todo: try extract size from length proxy
    size_t array_size = 1;

    for (const auto& prop : other_props)
    {
        if (prop.m_RecvType != prop.m_RecvType) // todo: check is name still same after this (because previously we store this name without array braces)
            break;

        // name.starts_with(real_prop_name)
        if (std::memcmp(prop.m_pVarName, real_prop_name.data(), real_prop_name.size()) != 0)
            break;

        // name.size() == real_prop_name.size()
        if (prop.m_pVarName[real_prop_name.size()] != '\0')
            break;

        ++array_size;
    }

    return {real_prop_name, array_size};
}

static void _Parse_client_class(storage& root_tree, netvar_table& tree, RecvTable* const recv_table, const size_t offset)
{
    const auto [props_begin, props_end] = _Get_props_range(recv_table);

    for (auto itr = props_begin; itr != props_end; ++itr)
    {
        const auto& prop = *itr;
        fds_assert(prop.m_pVarName != nullptr);
        const std::string_view prop_name = prop.m_pVarName;
        if (_Can_skip_netvar(prop_name))
            continue;

        const auto real_prop_offset = offset + prop.m_Offset;

        if (prop_name.rfind(']') != prop_name.npos)
        {
            const auto array_info = _Parse_prop_array(prop_name, {itr + 1, props_end}, tree);
            if (array_info.size > 0)
            {
                tree.add(real_prop_offset, itr, array_info.size, array_info.name);
                itr += array_info.size - 1;
            }
        }
        else if (prop.m_RecvType != DPT_DataTable)
        {
            tree.add(real_prop_offset, itr, 0, prop_name);
        }
        else if (prop.m_pDataTable && !prop.m_pDataTable->props.empty())
        {
            _Parse_client_class(root_tree, tree, prop.m_pDataTable, real_prop_offset);
        }
    }
}

void storage::iterate_client_class(ClientClass* root_class)
{
    for (auto client_class = root_class; client_class != nullptr; client_class = client_class->pNext)
    {
        const auto recv_table = client_class->pRecvTable;
        if (!recv_table || recv_table->props.empty())
            continue;

        hashed_string class_name = _Correct_class_name(client_class->pNetworkName);
        fds_assert(this->find(class_name));
        const auto added = this->add(std::move(class_name), true);

        _Parse_client_class(*this, *added, recv_table, 0);

        /*if (added->empty( ))
            this->erase(added.data( ));*/
    }
}

static void _Parse_datamap(netvar_table& tree, datamap_t* const map)
{
    for (auto& desc : map->data)
    {
        if (desc.fieldType == FIELD_EMBEDDED)
        {
            if (desc.TypeDescription != nullptr)
                fds_assert("Embedded datamap detected");
        }
        else if (desc.fieldName != nullptr)
        {
            const std::string_view name = desc.fieldName;
            if (_Can_skip_netvar(name))
                continue;
            tree.add(static_cast<size_t>(desc.fieldOffset[TD_OFFSET_NORMAL]), std::addressof(desc), 0, name);
        }
    }
}

void storage::iterate_datamap(datamap_t* const root_map)
{
    for (auto map = root_map; map != nullptr; map = map->baseMap)
    {
        if (map->data.empty())
            continue;

        hashed_string class_name = _Correct_class_name(map->dataClassName);
        const auto added         = this->add(std::move(class_name));

        _Parse_datamap(*added, map);

        /*if (added->empty( ))
            this->erase(added.data( ));*/
    }
}
