#include <fd/netvars/info.h>
#include <fd/netvars/storage.h>
#include <fd/netvars/type_resolve.h>

#include <nlohmann/json.hpp>
#include <nlohmann/ordered_map.hpp>

#include <filesystem>
#include <fstream>

#include <spdlog/spdlog.h>

namespace std::filesystem
{
static_assert(sizeof(path) == sizeof(std::wstring));

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
} // namespace std::filesystem

namespace fs = std::filesystem;

template <size_t S>
struct fmt::formatter<fmt::basic_memory_buffer<char, S>> : formatter<string_view>
{
    template <class C>
    auto format(basic_memory_buffer<char, S> const& buff, C& ctx) const
    {
        return formatter<string_view>::format(string_view(buff.data(), buff.size()), ctx);
    }
};

namespace fd
{
static bool _file_already_written(wchar_t const* fullPath, const std::span<char const> buffer)
{
    std::ifstream fileStored;
    fileStored.rdbuf()->pubsetbuf(nullptr, 0); // disable buffering
    fileStored.open(fullPath, std::ios::binary | std::ios::ate);

    if (!fileStored)
        return false;

    auto const size = fileStored.tellg();
    if (size != buffer.size())
        return false;

#if 0
	const unique_ptr<char[]> buff = new char[size];
	if (!file_stored.read(buff.get(), size))
		return false;
	return std::memcmp(buff.get(), buffer.data(), size) == 0;
#else
    using it_t = std::istream_iterator<char>;
    return equal<it_t>(fileStored, {}, buffer.begin());
#endif
}

#if 0
static void _Correct_path(std::string& path)
{
    std::replace(path.begin(), path.end(), fs::unpreferred_separator, fs::preferred_separator);
    if (!path.ends_with(fs::preferred_separator))
        path.push_back(fs::preferred_separator);
}

[[nodiscard]] static std::string _Correct_path(std::string_view const path)
{
    auto size = path.size();
    if (!path.ends_with(fs::preferred_separator) && !path.ends_with(fs::unpreferred_separator))
        ++size;

    std::string buff;
    buff.reserve(size);

    for (auto c : path)
        buff.push_back(c == fs::unpreferred_separator ? fs::preferred_separator : c);
    if (!path.ends_with(fs::preferred_separator))
        buff.push_back(fs::preferred_separator);

    return buff;
}
#endif

static void _write_to_file(wchar_t const* path, const std::span<char const> buff)
{
    // std::ofstream(full_path).write(buff.data(), buff.size());
    FILE* f;
    _wfopen_s(&f, path, L"w");
    _fwrite_nolock(buff.data(), 1, buff.size(), f);
    _fclose_nolock(f);
}

netvars_log::netvars_log() = default;

netvars_log::~netvars_log()
{
    if (dir.empty())
        return;
    //_Correct_path(dir);

    if (!fs::exists(dir) && !fs::create_directories(dir))
        return;
    fmt::wmemory_buffer fullPath;
    fullPath.append(dir);
    fullPath.append(file.name);
    fullPath.append(file.extension);
    fullPath.push_back(0);
    if (_file_already_written(fullPath.data(), buff))
        return;
    _write_to_file(fullPath.data(), buff);
}

netvars_classes::~netvars_classes()
{
    if (dir.empty())
        return;

    //_Correct_path(dir);

    struct path_info
    {
        fs::path              path;
        std::span<char const> buff;
    };

    auto const buildPath = [&](file_info const& info) -> path_info
    {
        // ReSharper disable CppRedundantCastExpression
        return { reinterpret_cast<const fs::path&>(dir) / reinterpret_cast<const fs::path&>(info.name), info.data };
        // ReSharper restore CppRedundantCastExpression
    };

    auto const skipWritten = [&](path_info const& p)
    {
        return _file_already_written(p.path.c_str(), p.buff);
    };

    auto const writeBuffer = [&](path_info const& p)
    {
        _write_to_file(p.path.c_str(), p.buff);
    };

    if (fs::create_directories(dir) || fs::is_empty(dir))
    {
        for (auto& f : files)
        {
            writeBuffer(buildPath(f));
        }
    }
    else
    {
        for (auto& f : files)
        {
            auto const info = buildPath(f);
            if (skipWritten(info))
                continue;
            writeBuffer(info);
        }
    }
}

netvars_classes::netvars_classes() = default;

//----

[[maybe_unused]]
static auto _correct_class_name(std::string_view const name)
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
        auto const c = *++name;
        if (c == '.')
            return true;
        if (c == '\0')
            return false;
    }
}

static bool _can_skip_netvar(std::string_view const name)
{
    return name.contains('.');
}

// #define GENERATE_STRUCT_MEMBERS
#define MERGE_DATA_TABLES

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
    auto propName = std::string(prop->name);
    for (auto& c : propName)
        c = std::tolower(c);
    return propName.contains("length") && propName.contains("proxy");
}

void netvars_storage::request_sort(netvar_table const* table)
{
    auto const idx = std::distance<netvar_table const*>(data_.data(), table);
    sortRequested_.push_back(idx);
}

void netvars_storage::sort()
{
    auto const sortEnd = std::unique(sortRequested_.begin(), sortRequested_.end());
    for (auto it = sortRequested_.begin(); it != sortEnd; ++it)
        data_[*it].sort();
    sortRequested_.clear();
}

void netvars_storage::add(netvar_table&& table)
{
#ifdef MERGE_DATA_TABLES
    assert(!this->find(table.name()));
#endif
    data_.emplace_back(std::move(table));
}

netvar_table* netvars_storage::add(std::string&& name, bool const root)
{
#ifdef MERGE_DATA_TABLES
    assert(!this->find(name));
#endif
    return &data_.emplace_back(std::move(name), root);
}

netvar_table* netvars_storage::add(std::string_view const name, bool const root)
{
#ifdef MERGE_DATA_TABLES
    assert(!this->find(name));
#endif
    return &data_.emplace_back(std::string(name), root);
}

netvars_storage::netvars_storage()
{
#ifdef _DEBUG
    // to force assert in get_offset
    sortRequested_.reserve(1);
#endif
    (void)this;
}

template <class T>
static auto _find_name(T& rng, std::string_view const name) -> decltype(rng.data())
{
    for (auto& item : rng)
    {
        if (item.name() == name)
            return &item;
    }

    return nullptr;
}

netvar_table* netvars_storage::find(std::string_view const name)
{
    return _find_name(data_, name);
}

netvar_table const* netvars_storage::find(std::string_view const name) const
{
    return _find_name(data_, name);
}

static auto _get_props_range(valve::recv_table const* recvTable)
{
    auto const& rawProps = recvTable->props;

    auto front = &rawProps.front();
    auto back  = &rawProps.back();

    if (_is_base_class(front))
        ++front;
    if (_is_length_proxy(back))
        --back;

    return std::pair(front, back + 1);
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
    std::string_view const  propName)
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
    assert(dynamic_cast<netvar_info const*>(info));
    return static_cast<netvar_info const*>(info)->array_size();
}

static size_t _store(
    valve::recv_prop const* arrayStart,
    std::string_view const  propName,
    valve::recv_prop const* arrayEnd,
    netvar_table*           customTable,
    size_t                  extraOffset = 0)
{
    assert(arrayStart->type != valve::DPT_DataTable); // Array DataTable detected!

    if (!propName.ends_with("[0]"))
    {
        auto const realPropName = propName.substr(0, propName.find('['));
        return _get_array_size(arrayStart, arrayEnd, realPropName);
    }

    auto const realPropName = propName.substr(0, propName.size() - 3);
    assert(!realPropName.contains(']'));

    if (auto const found = customTable->find(realPropName); found != nullptr)
        return _get_array_size(found);

    auto const arraySize = _get_array_size(arrayStart, arrayEnd, realPropName);
    customTable->add(new netvar_info(arrayStart->offset + extraOffset, arrayStart, arraySize, realPropName));

    return arraySize;
}

static void _store(
    valve::recv_prop const* prop,
    std::string_view const  propName,
    netvar_table*           customTable,
    size_t                  extraOffset = 0)
{
    if (auto found = customTable->find(propName); found != nullptr)
    {
        (void)found;
        return;
    }

    customTable->add(new netvar_info(prop->offset + extraOffset, prop, 0, propName));
}

static size_t _store(
    valve::recv_prop const*  arrayStart,
    valve::recv_table const* recvTable,
    valve::recv_prop const*  arrayEnd,
    netvar_table*            customTable,
    size_t                   extraOffset = 0)
{
    assert(arrayStart->type != valve::DPT_DataTable); // not implemented

    std::string_view const tableName(recvTable->name);
    if (auto const found = customTable->find(tableName); found != nullptr)
        return _get_array_size(found);

    auto const arraySize = _get_array_size(arrayStart, arrayEnd);
    customTable->add(new netvar_info(arrayStart->offset + extraOffset, arrayStart, arraySize, tableName));
    return arraySize;
}

// merge data tables
static void _parse(valve::recv_table const* recvTable, netvar_table* customTable, size_t rootOffset = 0)
{
    auto [propsBegin, propsEnd] = _get_props_range(recvTable);

    if (propsBegin == propsEnd)
        return;

    if (propsBegin->name[0] == '0')
        propsBegin += _store(propsBegin, recvTable, propsEnd, customTable, rootOffset);

    for (auto prop = propsBegin; prop != propsEnd; ++prop)
    {
        std::string_view const propName(prop->name);
        if (_can_skip_netvar(propName))
            continue;
        if (prop->type != valve::DPT_DataTable)
        {
            if (propName.ends_with(']'))
                prop += _store(prop, propName, propsEnd, customTable, rootOffset) - 1;
            else
                _store(prop, propName, customTable, rootOffset);
        }
        else if (auto const dt = prop->data_table; dt && !dt->props.empty())
        {
            _parse(dt, customTable, rootOffset + prop->offset);
        }
    }
}

static std::string_view _correct_recv_name(std::string_view const name)
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

// store all data tables
static void _parse(valve::recv_table const* recvTable, netvars_storage* storage)
{
    assert("WIP");

    /*const auto [propsBegin, propsEnd] = _get_props_range(recvTable);
    if (std::distance(propsBegin, propsEnd) == 0)
        return;
    const auto tableName = _correct_recv_name(recvTable->name);
    if (storage->find(tableName))
        return;
    const auto customTable = storage->add(tableName, recvTable->in_main_list);
    for (auto prop = propsBegin; prop != propsEnd; ++prop)
    {
        std::string_view const propName(prop->name);
        if (_can_skip_netvar(propName))
            continue;
        if (prop->type != valve::DPT_DataTable)
        {
            if (propName.ends_with(']'))
                prop += _store(prop, propName, propsEnd, customTable);
            else
                _store(prop, propName, customTable);
        }
        else if (const auto dt = prop->data_table; dt && !dt->props.empty())
        {
            _parse(dt, storage);
        }
    }*/
}

void netvars_storage::iterate_client_class(valve::client_class const* clientClass)
{
    assert(data_.empty()); // Iterate recv tables first!
    for (; clientClass != nullptr; clientClass = clientClass->next)
    {
        if (!clientClass->table)
            continue;
        if (clientClass->table->props.empty())
            continue;
#ifdef MERGE_DATA_TABLES
        netvar_table tmp(_correct_class_name(clientClass->name), true);
        _parse(clientClass->table, &tmp);
        if (tmp.begin() != tmp.end())
            this->add(std::move(tmp));
#else
        _parse(clientClass.table, this);
#endif
    }

    spdlog::info("[NETVARS] {} data tables stored", data_.size());
}

static auto _parse(valve::data_map const* rootMap, netvars_storage* storage)
{
    size_t updatedCount = 0;
    for (auto map = rootMap; map != nullptr; map = map->base)
    {
        if (map->data.empty())
            continue;

        std::string className;
#ifdef MERGE_DATA_TABLES
        className = _correct_class_name(map->name);
#else
        auto const nameBegin = static_cast<char const*>(memchr(map->name, '_', std::numeric_limits<size_t>::max()));
        write_std::string(className, "DT_", nameBegin);
        _correct_recv_name(className);
#endif
        auto       table      = storage->find(className);
        auto const tableAdded = table;
        if (!table)
            table = storage->add(std::move(className), true);
        else
            storage->request_sort(table);

        auto const tableEnd = table->end();
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
                auto const name = std::string_view(desc.name);
                if (!tableAdded && table->find(name)) // todo: check & correct type
                    continue;
                // fieldOffset[TD_OFFSET_NORMAL], array replaced with two varaibles
                table->add(new netvar_info(static_cast<size_t>(desc.offset), &desc, 0, name));
            }
        }

        if (tableEnd != table->end())
            ++updatedCount;
    }
    return updatedCount;
}

void netvars_storage::iterate_datamap(valve::data_map const* rootMap)
{
    assert(!data_.empty()); // Iterate datamap after recv tables!

    auto const result = _parse(rootMap, this);

    spdlog::info("[NETVARS] data map stored ({} classes updated)", result);
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

void netvars_storage::log_netvars(netvars_log& data)
{
    using json_type = nlohmann::ordered_json;

    assert(!data.dir.empty());
    assert(!data.file.name.empty());
    assert(!data.file.extension.empty());

    this->sort();

    json_type jsRoot;

    for (auto const& table : data_)
    {
        assert(!table.empty());

        jsRoot.push_back(table.name());
        auto& buff = jsRoot.back();

        for (auto const& info : table)
        {
            // using str_t = json_type::string_t;

            auto const name   = info.name();
            auto const type   = info.type();
            auto const offset = info.offset();

            if (type.empty())
            {
                buff.push_back({
                    {  "name",   name},
                    {"offset", offset}
                });
            }
            else
            {
                buff.push_back({
                    {  "name", (name)},
                    {"offset", offset},
                    {  "type", (type)}
                });
            }
        }
    }

    namespace jd = nlohmann::detail; // NOLINT(misc-unused-alias-decls)
    data.buff.reserve(1024 * 210);   // filse size ~210 kb
    jd::serializer<json_type>(jd::output_adapter(data.buff), data.filler)
        .dump(jsRoot, data.indent > 0, false, data.indent);
    data.buff.shrink_to_fit();
    spdlog::debug("[NETVARS] log buffer size: {}kb", data.buff.size() / 1024);
    spdlog::info(L"[NETVARS] log will be written to {}/{}{}", data.dir, data.file.name, data.file.extension);
}

struct generate_info
{
    std::string_view                   name;
    fmt::basic_memory_buffer<char, 32> typeOut;
    fmt::basic_memory_buffer<char, 64> typeCast;

    generate_info(basic_netvar_info const& info)
    {
        auto const typeRaw = info.type();
        if (typeRaw.empty())
            return;

        auto const isPointer   = typeRaw.ends_with('*');
        auto const typeDecayed = isPointer ? typeRaw.substr(0, typeRaw.size() - 1) : typeRaw;

        typeOut.append(typeDecayed);
        typeOut.push_back(isPointer ? '*' : '&');

        using namespace std::string_view_literals;

        if (isPointer)
            typeCast.push_back('*');
        typeCast.append("reinterpret_cast<"sv);
        typeCast.append(typeDecayed);
        typeCast.append("*>"sv);

        name = info.name();
    }
};

void netvars_storage::generate_classes(netvars_classes& data)
{
    assert(!data.dir.empty());

    this->sort();

    using namespace std::string_view_literals;

#ifdef GENERATE_STRUCT_MEMBERS
#error "not implemented"
#else

    data.files.resize(data_.size() * 2);
    auto file = data.files.begin();

    std::vector<char> source;
    auto              sourceInserter = std::back_insert_iterator(source);

    std::vector<char> header;
    auto              headerInserter = std::back_insert_iterator(header);

    std::vector<generate_info> table;

    constexpr auto _formatOff = "// clang-format off\n"
                                "// ReSharper disable All\n\n"sv;

    for (auto const& rawTable : data_)
    {
        assert(!rawTable.empty());

        table.clear();
        table.reserve(rawTable.size());
        for (auto& i : rawTable)
        {
            if (!i.type().empty())
                table.emplace_back(i);
        }

        auto const className = rawTable.name();

        source.clear();
        {
            fmt::basic_memory_buffer<char, 32> offsetsClass;
            offsetsClass.append(className);
            offsetsClass.append("_offsets"sv);

            source.append_range(_formatOff);
            // source.append_range(_assertGap);
            source.append_range("static struct\n{\n"sv);
            {
                for (auto& i : table)
                    fmt::format_to(sourceInserter, "\tsize_t {} = -1;\n", i.name);

                source.append_range("\n\tvoid init()\n\t{\n"sv);
                {
                    for (auto& i : table)
                    {
                        fmt::format_to(
                            sourceInserter,
                            "\t\tassert({name} == -1, \"already set!\";\n"
                            "\t\t{name} = get_netvars_storage()->get_offset(\'{className}\', \'{name}\');\n",
                            fmt::arg("name", i.name),
                            fmt::arg("className", className));
                    }
                }
                source.append_range("\t}\n"sv);
            }
            source.push_back('}');
            fmt::format_to(sourceInserter, " {};\n\n", offsetsClass);

            for (auto& i : table)
            {
                fmt::format_to(
                    sourceInserter,
                    "{typeOut} {className}::{name}()\n"
                    "{\n"
                    "\tassert({offsetsClass}.{name} != -1, \"not set!\");\n"
                    "\tconst auto addr = reinterpret_cast<uintptr_t>(this) + {offsetsClass}.{name};\n"
                    "\treturn {}(addr);\n",
                    "}\n",
                    fmt::arg("typeOut", i.typeOut),
                    fmt::arg("className", className),
                    fmt::arg("name", i.name),
                    fmt::arg("offsetsClass", offsetsClass),
                    i.typeCast);
            };
        }

        header.clear();
        {
            header.append_range(_formatOff);
            for (auto& i : table)
            {
                fmt::format_to(headerInserter, "{} {}():\n", i.typeOut, i.name);
            };
        }

        auto const storeFile = [&](std::string_view extension, std::span<char const> buff)
        {
            file->name.append_range(className).append_range(extension);
            file->data.append_range(buff);
            ++file;
        };

        storeFile("_h", header);
        storeFile("_cpp", source);

        // todo: if !MERGE_DATA_TABLES include wanted files!
    }

#undef table

#endif

    spdlog::info(L"[NETVARS] {} classes written to {}", data.files.size(), data.dir);
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

void netvars_storage::finish()
{
    sortRequested_.clear();
    sortRequested_.shrink_to_fit();

    spdlog::info("[NETVARS] {} classes stored", data_.size());
}

size_t netvars_storage::get_offset(std::string_view const className, std::string_view const name) const
{
    assert(sortRequested_.capacity() == 0); // "finish not called!"
    auto const offset = this->find(className)->find(name)->offset();
    spdlog::info("[NETVARS] {}->{} loaded", className, name);
    return offset;
}

void netvars_storage::clear()
{
    assert(sortRequested_.capacity() == 0); // "finish not called!"
    data_.clear();
    data_.shrink_to_fit();
}
} // namespace fd