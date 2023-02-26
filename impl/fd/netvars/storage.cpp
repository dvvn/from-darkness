#include <fd/netvars/info.h>
#include <fd/netvars/storage.h>
#include <fd/netvars/type_resolve.h>

#include <boost/filesystem.hpp>

#include <spdlog/spdlog.h>

namespace boost::filesystem
{
static_assert(std::same_as<path::string_type, std::wstring>);

bool exists(std::wstring const& dir)
{
    return exists(reinterpret_cast<path const&>(dir));
}

bool create_directory(std::wstring const& dir)
{
    return create_directory(reinterpret_cast<path const&>(dir));
}

bool create_directories(std::wstring const& dir)
{
    return create_directories(reinterpret_cast<path const&>(dir));
}

bool is_empty(std::wstring const& dir)
{
    return is_empty(reinterpret_cast<path const&>(dir));
}

// constexpr auto             preferred_separator   = path::preferred_separator;
// constexpr path::value_type unpreferred_separator = preferred_separator == '\\' ? '/' : '\\';
} // namespace boost::filesystem

template <>
struct fmt::formatter<boost::filesystem::path, wchar_t> : formatter<wstring_view, wchar_t>
{
    auto format(boost::filesystem::path const& p, wformat_context& ctx) const
    {
        return formatter<wstring_view, wchar_t>::format(p.native(), ctx);
    }
};

template <>
struct fmt::formatter<fd::netvars_log, wchar_t> : formatter<boost::filesystem::path, wchar_t>
{
    auto format(fd::netvars_log const& l, wformat_context& ctx) const
    {
        return formatter<boost::filesystem::path, wchar_t>::format(l.make_path(), ctx);
    }
};

namespace fs = boost::filesystem;

// #define GENERATE_STRUCT_MEMBERS

namespace fd
{
using data_map_ptr   = valve::data_map const*;
using recv_table_ptr = valve::recv_table const*;

//----

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

static bool _can_skip_netvar(char const* name)
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

static auto _is_base_class(valve::recv_prop const* prop)
{
    constexpr auto str = std::string_view("baseclass");
    // return std::memcmp(prop->name, str.data(), str.size()) == 0 && prop->name[str.size()] == '\0';
    return prop->name == str;
}

static auto _is_length_proxy(valve::recv_prop const* prop)
{
    if (prop->array_length_proxy)
        return true;

    constexpr auto lp = std::string_view("lengthproxy");
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

static size_t _get_array_size(valve::recv_prop const* arrayStart, valve::recv_prop const* arrayEnd)
{
    size_t arraySize = 1;
    for (auto it = arrayStart + 1; it != arrayEnd; ++it)
    {
        if (arrayStart->type != it->type)
            break;
        ++arraySize;
    }

    return arraySize;
}

static size_t _get_array_size(
    valve::recv_prop const* arrayStart,
    valve::recv_prop const* arrayEnd,
    std::string_view        propName)
{
    // todo: try extract size from length proxy

    size_t arraySize = 1;
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

static size_t _get_array_size(basic_netvar_info const* info)
{
    assert(dynamic_cast<netvar_info const*>(info) != nullptr);
    return static_cast<netvar_info const*>(info)->array_size();
}

static size_t _store(
    valve::recv_prop const* arrayStart,
    std::string_view        propName,
    valve::recv_prop const* arrayEnd,
    netvar_table*           netvarTable,
    size_t const            extraOffset = 0)
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
    netvarTable->add(new netvar_info(arrayStart->offset + extraOffset, arrayStart, realPropName, arraySize));

    return arraySize;
}

static void _store(
    valve::recv_prop const* prop,
    std::string_view        propName,
    netvar_table*           netvarTable,
    size_t const            extraOffset = 0)
{
    if (auto found = netvarTable->find(propName); found != nullptr)
    {
        (void)found;
        return;
    }

    netvarTable->add(new netvar_info(prop->offset + extraOffset, prop, propName));
}

// wrap datatable in std::array
static size_t _store(
    valve::recv_prop const* arrayStart,
    recv_table_ptr          recvTable,
    valve::recv_prop const* arrayEnd,
    netvar_table*           netvarTable,
    size_t const            extraOffset = 0)
{
    assert(arrayStart->type != valve::DPT_DataTable); // not implemented

    auto tableName = std::string_view(recvTable->name);
    if (auto found = netvarTable->find(tableName); found != nullptr)
        return _get_array_size(found);

    auto arraySize = _get_array_size(arrayStart, arrayEnd);
    assert(arraySize != 0);
    netvarTable->add(new netvar_info(arrayStart->offset + extraOffset, arrayStart, tableName, arraySize));
    return arraySize;
}

// wrap datatable in std::array<type>
// type mut be manually generated
static bool _store(
    valve::recv_prop const* arrayStart,
    recv_table_ptr          recvTable,
    valve::recv_prop const* arrayEnd,
    netvar_table*           netvarTable,
    std::string_view        type,
    size_t const            extraOffset)
{
    assert(arrayStart->type == valve::DPT_DataTable);

    auto tableName = std::string_view(recvTable->name);
    if (auto found = netvarTable->find(tableName); found != nullptr)
        return 0;

    auto arraySize = _get_array_size(arrayStart, arrayEnd);
    assert(std::distance(arrayStart, arrayEnd) == arraySize);
    assert(arraySize != 0);
    netvarTable->add(new netvar_info_instant(arrayStart->offset + extraOffset, tableName, type, arraySize));
    return 1;
}

// merge data tables
static void _parse(
    recv_table_ptr recvTable,
    netvar_table*  netvarTable,
    netvar_tables& internalStorage,
    size_t         rootOffset = 0);

static void _parse_lproxy(
    valve::recv_prop const* proxy,
    valve::recv_prop const* prop,
    recv_table_ptr          recvTable,
    valve::recv_prop const* propsEnd,
    netvar_table*           netvarTable,
    netvar_tables&          internalStorage,
    size_t                  rootOffset)
{
    if (prop->type != valve::DPT_DataTable)
    {
        _store(prop, recvTable, propsEnd, netvarTable, rootOffset);
    }
    else if (auto dt = prop->data_table; dt && !dt->props.empty())
    {
        auto tableName = std::string_view(dt->name);
        if (_store(prop, dt, propsEnd, netvarTable, tableName, rootOffset + proxy->offset))
        {
            // todo: return if internalStorage contains tableName

            auto internalTable = netvar_table(tableName);
            _parse(dt, &internalTable, internalStorage);
            internalStorage.add(std::move(internalTable));
        }
    }
}

// merge data tables
void _parse(recv_table_ptr recvTable, netvar_table* netvarTable, netvar_tables& internalStorage, size_t rootOffset)
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
        auto propName = std::string_view(prop->name);
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

static std::string_view _correct_recv_name(char const* name)
{
    return _correct_recv_name(std::string_view(name));
}

[[maybe_unused]]
static void _correct_recv_name(std::string const& name)
{
    (void)name;
}

#if 0
// store all data tables
static void _parse(recv_table_ptr recvTable, netvar_tables_ordered& storage)
{
    assert("WIP");

    /*const auto [propsBegin, propsEnd] = _get_props_range(recvTable);
    if (std::distance(propsBegin, propsEnd) == 0)
        return;
    const auto tableName = _correct_recv_name(recvTable->name);
    if (storage.find(tableName))
        return;
    const auto netvarTable = storage.add(tableName, recvTable->in_main_list);
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
        else if (const auto dt = prop->data_table; dt && !dt->props.empty())
        {
            _parse(dt, storage);
        }
    }*/
}
#endif

void netvars_storage::iterate_client_class(valve::client_class const* cclass)
{
    assert(data_.empty()); // Iterate recv tables first!
    for (; cclass != nullptr; cclass = cclass->next)
    {
        if (!cclass->table)
            continue;
        if (cclass->table->props.empty())
            continue;
#ifdef FD_NETVARS_DT_MERGE
        auto tmp = netvar_table(_correct_class_name(cclass->name));
        _parse(cclass->table, &tmp, internal_);
        if (!tmp.empty())
            data_.add(std::move(tmp));
#else
#error "not implemented"
            //_parse(cclass->table, data_, internal_);
#endif
    }

    spdlog::info("[NETVARS] {} data tables stored", data_.size());
}

static void _parse(data_map_ptr map, netvar_tables_ordered& storage)
{
    for (; map != nullptr; map = map->base)
    {
        if (map->data.empty())
            continue;

        std::string className;
#ifdef FD_NETVARS_DT_MERGE
        className = _correct_class_name(map->name);
#else
        auto nameBegin = static_cast<char const*>(memchr(map->name, '_', std::numeric_limits<size_t>::max()));
        write_std::string(className, "DT_", nameBegin);
        _correct_recv_name(className);
#endif
        auto table      = storage.find(className);
        auto tableAdded = table;
        if (!tableAdded)
            table = storage.add(std::move(className));
        else
            storage.request_sort(table);

        for (auto& desc : map->data)
        {
            if (desc.type == valve::FIELD_EMBEDDED)
            {
                // not implemented
                if (desc.description)
                    assert("Embedded datamap detected");
            }
            else if (desc.name)
            {
                if (_can_skip_netvar(desc.name))
                    continue;
                auto name = std::string_view(desc.name);
                if (!tableAdded && table->find(name)) // todo: check & correct type
                    continue;
                // fieldOffset[TD_OFFSET_NORMAL], array replaced with two varaibles
                table->add(new netvar_info(static_cast<size_t>(desc.offset), &desc, name));
            }
        }
    }
}

void netvars_storage::iterate_datamap(data_map_ptr rootMap)
{
    assert(!data_.empty()); // Iterate datamap after recv tables!
    _parse(rootMap, data_);
    spdlog::info("[NETVARS] data map stored!");
}

#if 0
void netvars_storage::store_handmade_netvars()
{
    const auto baseent = this->find("C_BaseEntity");
    this->request_sort(baseent);
    baseent->add<var_map>(0x24, "m_InterpVarMap");
    baseent->add<matrix3x4>(SIG(client, "8B 55 ? 85 D2 74 23 8B 87 ? ? ? ? 8B 4D ? 3B C8", plus(9), deref<1>(), minus(8)), "m_BonesCache", extract_type_valve_vector);

    const auto baseanim = this->find("C_BaseAnimating");
    this->request_sort(baseanim);
    // m_vecRagdollVelocity - 128
    baseanim->add<animation_layer>(SIG(client, "8B 87 ? ? ? ? 83 79 04 00 8B", plus(2), deref<1>()), "m_AnimOverlays", extract_type_valve_vector);
    // m_hLightingOrigin - 32
    baseanim->add<float>(SIG(client, "C7 87 ? ? ? ? ? ? ? ? 89 87 ? ? ? ? 8B 8F", plus(2), deref<1>()), "m_flLastBoneSetupTime");
    // ForceBone + 4
    baseanim->add<int>(SIG(client, "89 87 ? ? ? ? 8B 8F ? ? ? ? 85 C9 74 10", plus(2), deref<1>()), "m_iMostRecentModelBoneCounter");

    invoke(log, "netvars - handmade netvars stored");
}
#endif

//------

void netvars_storage::log_netvars(netvars_log& log)
{
    data_.sort();

    auto bytesWritten = log.fill(data_.make_updater());
    spdlog::debug("[NETVARS] log buffer size: {}kb", bytesWritten / 1024);
    spdlog::info(L"[NETVARS] log will be written to {}", log);
}

void netvars_storage::generate_classes(netvars_classes& data)
{
    data_.sort();

    (data.fill(data_.make_updater(), data_.size()));
    // todo: include internal files in data_! (made struct with provided include dirs)
    // todo: generate one .cpp file and include all _cpp internal files there!
    (data.fill(internal_.make_updater(), internal_.size()));
    spdlog::info(
        L"[NETVARS] {} classes, and {} internal classes written to {}",
        data_.size() * 2,
        internal_.size() * 2,
        data.dir);
}

//-------

#if 0
void netvars_storage::init_default()
{
    // run default initialization

    this->iterate_client_class(aOBJECT_GET(base_client)->GetAllClasses(), "All");

    using datamap_fn = data_map*(__thiscall*)(const void*);

    const auto baseent        = rt_modules::client.find_vtable<"C_BaseEntity">();
    const auto baseent_vtable = *reinterpret_cast<datamap_fn**>(baseent);
    this->iterate_datamap(invoke(baseent_vtable[15], baseent), "DataDescMap");
    this->iterate_datamap(invoke(baseent_vtable[17], baseent), "PredictionDescMap");

    this->store_handmade_netvars();
#ifdef _DEBUG
    this->log_netvars(*FD_OBJECT_GET(netvars_log));
    this->generate_classes(*FD_OBJECT_GET(netvars_classes));
#endif
}
#endif

size_t netvars_storage::get_offset(std::string_view className, std::string_view name) const
{
    auto offset = data_.find(className)->find(name)->offset();
    spdlog::info("[NETVARS] {}->{} loaded", className, name);
    return offset;
}

void netvars_storage::clear()
{
    netvars_storage empty;
    using std::swap;
    swap(*this, empty);
}
} // namespace fd