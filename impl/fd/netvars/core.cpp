#include "core.h"
#include "impl/info.h"
#include "impl/table.h"
#include "impl/tables.h"

#include <fd/lazy_invoke.h>
#include <fd/logging/default.h>
#include <fd/valve2/client.h>
#include <fd/valve2/entity.h>
#include <fd/valve2/recv_table.h>

#include <boost/filesystem.hpp>
#include <boost/nowide/fstream.hpp>
//
#include <fd/library_info.h>

#include <fmt/format.h>
#include <fmt/os.h>

#include <algorithm>

namespace boost::filesystem
{
static_assert(std::same_as<path::string_type, std::wstring>);

bool exists(std::wstring const &dir)
{
    return exists(reinterpret_cast<path const &>(dir));
}

bool create_directory(std::wstring const &dir)
{
    return create_directory(reinterpret_cast<path const &>(dir));
}

bool create_directories(std::wstring const &dir)
{
    return create_directories(reinterpret_cast<path const &>(dir));
}

bool is_empty(std::wstring const &dir)
{
    return is_empty(reinterpret_cast<path const &>(dir));
}
} // namespace boost::filesystem

template <>
struct fmt::formatter<boost::filesystem::path, wchar_t> : formatter<wstring_view, wchar_t>
{
    auto format(boost::filesystem::path const &p, wformat_context &ctx) const
    {
        return formatter<wstring_view, wchar_t>::format(p.generic_wstring(), ctx);
    }
};

// #define GENERATE_STRUCT_MEMBERS

namespace fd
{
[[maybe_unused]]
static auto _correct_class_name(std::string_view name)
{
    // fmt::basic_memory_buffer<char, 32> buff;
    std::string buff;
    if (name[0] == 'C' && name[1] != '_')
    {
        assert(std::isalnum(name[1]));
        // internal csgo classes looks like C_***
        // same classes in shared code look like C***
        buff.push_back('C');
        buff.push_back('_');
        buff.append(name.substr(1));
    }
    else
    {
        assert(!name.starts_with("DT_"));
        buff.append(name);
    }
    return buff;
}

static bool _can_skip_netvar(char const *name)
{
    for (;;)
    {
        auto c = *++name;
        if (c == '.')
            return true;
        if (c == '\0')
            return false;
    }
}

static bool _can_skip_netvar(std::string_view name)
{
    return name.contains('.');
}

static auto _is_base_class(valve::recv_prop *prop)
{
    constexpr std::string_view str = "baseclass";
    // return std::memcmp(prop->name, str.data(), str.size()) == 0 && prop->name[str.size()] == '\0';
    return prop->name == str;
}

static auto _is_length_proxy(valve::recv_prop *prop)
{
    if (prop->array_length_proxy)
        return true;

    constexpr std::string_view lp = "lengthproxy";
#if 1
    return prop->name == lp;
#else

    constexpr auto part1 = lp.substr(0, 6);
    constexpr auto part2 = lp.substr(6);

    auto propName = std::string_view(prop->name);

    auto propNameSize = propName.size();
    if (propNameSize < lp.size())
        return false;

    fmt::basic_memory_buffer<char, 32> buff;

    for (auto c : propName)
        buff.push_back(std::tolower(c));

    std::construct_at(&propName, buff.data(), propNameSize);
    return propNameSize == lp.size() ? propName == lp : (propName.contains(part1) && propName.contains(part2));
#endif
}

static auto _get_array_size(valve::recv_prop *arrayStart, valve::recv_prop *arrayEnd)
{
    uint16_t arraySize = 1;
    for (auto it = arrayStart + 1; it != arrayEnd; ++it)
    {
        if (arrayStart->type != it->type)
            break;
        ++arraySize;
    }

    return arraySize;
}

static auto _get_array_size(valve::recv_prop *arrayStart, valve::recv_prop *arrayEnd, std::string_view propName)
{
    // todo: try extract size from length proxy

    uint16_t arraySize = 1;
    for (auto it = arrayStart + 1; it != arrayEnd; ++it)
    {
        if (arrayStart->type != it->type)
            break;
        if (memcmp(propName.data(), it->name, propName.size()) != 0)
            break;
        ++arraySize;
    }

    return arraySize;
}

static auto _get_array_size(netvar_info const *info)
{
    return info->array_size();
}

static size_t _store(
    valve::recv_prop *arrayStart,
    std::string_view propName,
    valve::recv_prop *arrayEnd,
    netvar_table *netvarTable,
    size_t extraOffset = 0)
{
    assert(arrayStart->type != valve::DPT_DataTable); // Array DataTable detected!

    if (!propName.ends_with("[0]"))
    {
        auto realPropName = propName.substr(0, propName.find('['));
        return _get_array_size(arrayStart, arrayEnd, realPropName);
    }

    auto realPropName = propName.substr(0, propName.size() - 3);
    assert(!realPropName.contains(']'));

    if (auto found = netvarTable->find(realPropName); found != nullptr)
        return _get_array_size(found);

    auto arraySize = _get_array_size(arrayStart, arrayEnd, realPropName);
    netvarTable->emplace_back(arrayStart->offset + extraOffset, arraySize, arrayStart, realPropName);

    return arraySize;
}

static void _store(valve::recv_prop *prop, std::string_view propName, netvar_table *netvarTable, size_t extraOffset = 0)
{
    if (auto found = netvarTable->find(propName); found != nullptr)
    {
        (void)found;
        return;
    }

    netvarTable->emplace_back(prop->offset + extraOffset, prop, propName);
}

// wrap datatable in std::array
static size_t _store(
    valve::recv_prop *arrayStart,
    valve::recv_table *recvTable,
    valve::recv_prop *arrayEnd,
    netvar_table *netvarTable,
    size_t extraOffset = 0)
{
    assert(arrayStart->type != valve::DPT_DataTable); // not implemented

    std::string_view tableName = recvTable->name;
    if (auto found = netvarTable->find(tableName); found != nullptr)
        return _get_array_size(found);

    auto arraySize = _get_array_size(arrayStart, arrayEnd);
    assert(arraySize != 0);
    netvarTable->emplace_back(arrayStart->offset + extraOffset, arraySize, arrayStart, tableName);
    return arraySize;
}

// wrap datatable in std::array<type>
// type mut be manually generated
static bool _store(
    valve::recv_prop *arrayStart,
    valve::recv_table *recvTable,
    valve::recv_prop *arrayEnd,
    netvar_table *netvarTable,
    std::string_view type,
    size_t extraOffset)
{
    assert(arrayStart->type == valve::DPT_DataTable);

    std::string_view tableName = recvTable->name;
    if (auto found = netvarTable->find(tableName); found != nullptr)
        return false;

    auto arraySize = _get_array_size(arrayStart, arrayEnd);
    assert(std::distance(arrayStart, arrayEnd) == arraySize);
    assert(arraySize != 0);
    /*netvarTable->emplace_back(
        arrayStart->offset + extraOffset,
        netvar_type_array(arraySize, custom_netvar_type(type, fmt::format("<fd/valve/{}.h>", type))),
        tableName);*/
    return true;
}

// merge data tables
static void _parse(
    valve::recv_table *recvTable,
    netvar_table *netvarTable,
    netvar_tables &internalStorage,
    size_t rootOffset = 0);

static void _parse_lproxy(
    valve::recv_prop *proxy,
    valve::recv_prop *prop,
    valve::recv_table *recvTable,
    valve::recv_prop *propsEnd,
    netvar_table *netvarTable,
    netvar_tables &internalStorage,
    size_t rootOffset)
{
    if (prop->type != valve::DPT_DataTable)
    {
        _store(prop, recvTable, propsEnd, netvarTable, rootOffset);
    }
    else if (auto dt = prop->data_table; dt && !dt->props.empty())
    {
        std::string_view tableName = dt->name;
        if (!_store(prop, dt, propsEnd, netvarTable, tableName, rootOffset + proxy->offset))
            return;
        if (internalStorage.find(tableName))
            return;

        auto tmp = netvar_table(tableName);
        _parse(dt, &tmp, internalStorage);
        if (!tmp.empty())
            internalStorage.emplace_back(std::move(tmp));
    }
}

// merge data tables
void _parse(valve::recv_table *recvTable, netvar_table *netvarTable, netvar_tables &internalStorage, size_t rootOffset)
{
    auto prop     = recvTable->props.data();
    auto propsEnd = prop + recvTable->props.size();

    if (_is_base_class(prop))
    {
        if (recvTable->props.size() == 1)
            return;
        ++prop;
    }

    if (prop->name[0] == '0')
    {
        auto arraySize = _store(prop, recvTable, propsEnd, netvarTable, rootOffset);
        prop += arraySize;
    }
    else if (_is_length_proxy(prop))
    {
        _parse_lproxy(prop, prop + 1, recvTable, propsEnd, netvarTable, internalStorage, rootOffset);
        return;
    }

    for (; prop != propsEnd; ++prop)
    {
        std::string_view propName = prop->name;
        if (_can_skip_netvar(propName))
            continue;
        assert(!std::isdigit(propName[0]));
        if (prop->type != valve::DPT_DataTable)
        {
            if (propName.ends_with(']'))
                prop += _store(prop, propName, propsEnd, netvarTable, rootOffset) - 1;
            else
                _store(prop, propName, netvarTable, rootOffset);
        }
        else if (auto dt = prop->data_table; dt && !dt->props.empty())
        {
            _parse(dt, netvarTable, internalStorage, rootOffset + prop->offset);
        }
    }
}

static std::string_view _correct_recv_name(std::string_view name)
{
    // reserved
    return name;
}

static std::string_view _correct_recv_name(char const *name)
{
    return _correct_recv_name(std::string_view(name));
}

[[maybe_unused]]
static void _correct_recv_name(std::string const &name)
{
    (void)name;
}

#if 0
// store all data tables
static void _parse(valve::recv_table * recvTable, netvar_tables_ordered& storage)
{
    assert(0&&"WIP");

    /*auto [propsBegin, propsEnd] = _get_props_range(recvTable);
    if (std::distance(propsBegin, propsEnd) == 0)
        return;
    auto tableName = _correct_recv_name(recvTable->name);
    if (storage.find(tableName))
        return;
    auto netvarTable = storage.add(tableName, recvTable->in_main_list);
    for (auto prop = propsBegin; prop != propsEnd; ++prop)
    {
        std::string_view propName(prop->name);
        if (_can_skip_netvar(propName))
            continue;
        if (prop->type != valve::DPT_DataTable)
        {
            if (propName.ends_with(']'))
                prop += _store(prop, propName, propsEnd, netvarTable);
            else
                _store(prop, propName, netvarTable);
        }
        else if (auto dt = prop->data_table; dt && !dt->props.empty())
        {
            _parse(dt, storage);
        }
    }*/
}
#endif

static netvar_tables internal_;
static netvar_tables_ordered data_;

void store_netvars(void *client_interface)
{
    assert(data_.empty()); // Iterate recv tables first!
    for (auto cclass : valve::client_class_range(client_interface))
    {
        if (!cclass.recv_table)
            continue;
        auto rtable = static_cast<valve::recv_table *>(cclass.recv_table);
        if (rtable->props.empty())
            continue;
#ifdef FD_NETVARS_DT_MERGE
        auto tmp = netvar_table(_correct_class_name(cclass.name));
        _parse(rtable, &tmp, internal_);
        if (!tmp.empty())
            data_.emplace_back(std::move(tmp));
#else
#error "not implemented"
            //_parse(cclass->table, data_, internal_);
#endif
    }
    default_logger->write<log_level::info>("[NETVARS] {} data tables stored", data_.size());
}

static void _parse(valve::data_map *map, netvar_tables_ordered &storage)
{
    for (; map != nullptr; map = map->base)
    {
        if (map->data.empty())
            continue;

        std::string class_name;
#ifdef FD_NETVARS_DT_MERGE
        class_name = _correct_class_name(map->name);
#else
        auto nameBegin = static_cast<char const *>(memchr(map->name, '_', std::numeric_limits<size_t>::max()));
        write_std::string(className, "DT_", nameBegin);
        _correct_recv_name(className);
#endif
        auto table      = storage.find(class_name);
        auto tableFound = table != nullptr;
        if (!tableFound)
            table = &storage.emplace_back(std::move(class_name));
        else
            storage.request_sort(table);

        for (auto &desc : map->data)
        {
            if (desc.type == valve::FIELD_EMBEDDED)
            {
                // not implemented
                if (desc.description)
                    assert(0 && "Embedded datamap detected");
            }
            else if (desc.name)
            {
                if (_can_skip_netvar(desc.name))
                    continue;
                std::string_view name = desc.name;
                if (tableFound && table->find(name)) // todo: check & correct type
                    continue;
                // fieldOffset[TD_OFFSET_NORMAL], array replaced with two varaibles
                table->emplace_back(static_cast<size_t>(desc.offset), &desc, name);
            }
        }
    }
}

void store_extra_netvars(void *entity)
{
    assert(!data_.empty()); // Iterate datamap after recv tables!
    _parse(valve::get_desc_data_map(entity), data_);
    _parse(valve::get_prediction_data_map(entity), data_);
    default_logger->write<log_level::info>("[NETVARS] data map stored!");
}

void store_custom_netvars(library_info const &client_dll)
{
#if 0
    auto baseent = this->find("C_BaseEntity");
    this->request_sort(baseent);
    baseent->add<var_map>(0x24, "m_InterpVarMap");
    baseent->add<matrix3x4>(SIG(client, "8B 55 ? 85 D2 74 23 8B 87 ? ? ? ? 8B 4D ? 3B C8", plus(9), deref<1>(), minus(8)), "m_BonesCache", extract_type_valve_vector);

    auto baseanim = this->find("C_BaseAnimating");
    this->request_sort(baseanim);
    // m_vecRagdollVelocity - 128
    baseanim->add<animation_layer>(SIG(client, "8B 87 ? ? ? ? 83 79 04 00 8B", plus(2), deref<1>()), "m_AnimOverlays", extract_type_valve_vector);
    // m_hLightingOrigin - 32
    baseanim->add<float>(SIG(client, "C7 87 ? ? ? ? ? ? ? ? 89 87 ? ? ? ? 8B 8F", plus(2), deref<1>()), "m_flLastBoneSetupTime");
    // ForceBone + 4
    baseanim->add<int>(SIG(client, "89 87 ? ? ? ? 8B 8F ? ? ? ? 85 C9 74 10", plus(2), deref<1>()), "m_iMostRecentModelBoneCounter");

#endif
}

void create_netvar_classes(std::wstring_view dir)
{
    data_.sort();

    using boost::filesystem::path;
    using boost::nowide::fstream;

    path root_dir = dir;
    if (!create_directories(root_dir))
        assert(exists(root_dir));
    std::vector<char> buff;

    std::for_each(data_.begin(), data_.end(), [&](netvar_table const &table) noexcept {
        invoke_on_destruct reuse_prepare = [&] {
            auto &str = const_cast<path::string_type &>(root_dir.native());
            str.erase(dir.size());
            buff.clear();
        };

        auto table_name = table.name();

        auto file = fstream(root_dir.append(table_name), std::ios::binary);
        if (!file)
            std::unreachable();

        fmt::format_to(std::back_inserter(buff), "namespace {}\n", table_name);
        buff.push_back('}');
        buff.push_back('\n');
        std::for_each(table.begin(), table.end(), [&](netvar_info const &info) noexcept {
            fmt::format_to(
                std::back_inserter(buff),
                "auto {name} = netvar_getter(std::in_place_type<{type}>, {source}, {name});\n",
                fmt::arg("source", table_name),
                fmt::arg("name", info.name()),
                fmt::arg("type", info.type()));
        });
        buff.push_back('}');

        file.rdbuf()->pubsetbuf(buff.data(), buff.size());
    });

#if 0
    _fill(data, data_);
    // todo: include internal files in data_! (made struct with provided include dirs)
    // todo: generate one .cpp file and include all _cpp internal files there!
    _fill(data, internal_);
    default_logger->write<log_level::info>(
        L"[NETVARS] {} classes, and {} internal classes written to {}",
        data_.size() * 2,
        internal_.size() * 2,
        data.dir);
#endif
}

void dump_netvars(std::wstring_view dir)
{
    data_.sort();

#if 0
    _fill(log, data_);
     default_logger->write<log_level::info>(L"[NETVARS] log will be written to {}", log);
#endif
}

size_t get_netvar_offset(netvar_tag class_name, netvar_tag name)
{
    auto offset = data_.find(class_name)->find(name)->offset();
    default_logger->write<log_level::info>("[NETVARS] {}->{} loaded", class_name, name);
    return offset;
}
} // namespace fd