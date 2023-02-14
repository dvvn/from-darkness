#include <fd/assert.h>
#include <fd/format.h>
#include <fd/functional.h>
#include <fd/json.h>
#include <fd/logger.h>
#include <fd/netvar_storage_impl.h>
#include <fd/netvar_type_resolve.h>
#include <fd/string_info.h>
#include <fd/utility.h>
#include <fd/views.h>

#include <filesystem>
#include <fstream>

#if 1
namespace std::filesystem
{
static_assert(sizeof(path) == sizeof(fd::wstring));

bool exists(const fd::wstring& dir)
{
    return exists(reinterpret_cast<const path&>(dir));
}

bool create_directory(const fd::wstring& dir)
{
    return create_directory(reinterpret_cast<const path&>(dir));
}

bool create_directories(const fd::wstring& dir)
{
    return create_directories(reinterpret_cast<const path&>(dir));
}

bool is_empty(const fd::wstring& dir)
{
    return is_empty(reinterpret_cast<const path&>(dir));
}

constexpr auto             preferred_separator   = path::preferred_separator;
constexpr path::value_type unpreferred_separator = preferred_separator == '\\' ? '/' : '\\';
} // namespace std::filesystem

namespace fs = std::filesystem;
#endif

namespace fd
{
template <typename T>
static bool _file_already_written(const T& fullPath, const range_view<const char*> buffer)
{
    std::ifstream fileStored;
    fileStored.rdbuf()->pubsetbuf(nullptr, 0); // disable buffering
    fileStored.open(fullPath, std::ios::binary | std::ios::ate);

    if (!fileStored)
        return false;

    const auto size = fileStored.tellg();
    if (size != _size(buffer))
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
static void _Correct_path(wstring& path)
{
    std::replace(path.begin(), path.end(), fs::unpreferred_separator, fs::preferred_separator);
    if (!path.ends_with(fs::preferred_separator))
        path.push_back(fs::preferred_separator);
}

[[nodiscard]] static wstring _Correct_path(const wstring_view path)
{
    auto size = path.size();
    if (!path.ends_with(fs::preferred_separator) && !path.ends_with(fs::unpreferred_separator))
        ++size;

    wstring buff;
    buff.reserve(size);

    for (auto c : path)
        buff.push_back(c == fs::unpreferred_separator ? fs::preferred_separator : c);
    if (!path.ends_with(fs::preferred_separator))
        buff.push_back(fs::preferred_separator);

    return buff;
}
#endif

static void _write_to_file(const wstring_view path, const range_view<const char*> buff)
{
    // std::ofstream(full_path).write(buff.data(), buff.size());
    FILE* f;
    _wfopen_s(&f, _begin(path), L"w");
    _fwrite_nolock(_begin(buff), 1, _size(buff), f);
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
    const auto fullPath = make_string(dir, file.name, file.extension);
    if (_file_already_written(fullPath, buff))
        return;
    _write_to_file(fullPath, buff);
}

netvars_classes::~netvars_classes()
{
    if (dir.empty())
        return;

    //_Correct_path(dir);

    struct path_info
    {
        wstring                 path;
        range_view<const char*> buff;
    };

    const auto buildPath = [&](const file_info& info) -> path_info {
        return { make_string(dir, info.name), info.data };
    };

    const auto skipWritten = [&](const path_info& p) {
        return _file_already_written(p.path, p.buff);
    };

    const auto writeBuffer = [&](const path_info& p) {
        _write_to_file(p.path, p.buff);
    };

    if (fs::create_directories(dir) || fs::is_empty(dir))
    {
        for (auto& f : range_view(files))
        {
            writeBuffer(buildPath(f));
        }
    }
    else
    {
        for (auto& f : range_view(files))
        {
            const auto info = buildPath(f);
            if (skipWritten(info))
                continue;
            writeBuffer(info);
        }
    }
}

netvars_classes::netvars_classes() = default;

//----

[[maybe_unused]] static auto _correct_class_name(const string_view name)
{
    string buff;
    if (name[0] == 'C' && name[1] != '_')
    {
        FD_ASSERT(is_alnum(name[1]));
        // internal csgo classes looks like C_***
        // same classes in shared code look like C***
        write_string(buff, "C_", name.substr(1));
    }
    else
    {
        FD_ASSERT(!name.starts_with("DT_"));
        write_string(buff, name);
    }
    return buff;
}

static bool _can_skip_netvar(const char* name)
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

static bool _can_skip_netvar(const string_view name)
{
    return name.contains('.');
}

// #define GENERATE_STRUCT_MEMBERS
#define MERGE_DATA_TABLES

static auto _is_base_class(const valve::recv_prop* prop)
{
    constexpr string_view str = "baseclass";
    // return std::memcmp(prop->name, str.data(), str.size()) == 0 && prop->name[str.size()] == '\0';
    return prop->name == str;
};

static auto _is_length_proxy(const valve::recv_prop* prop)
{
    if (prop->array_length_proxy)
        return true;
    string propName(prop->name);
    for (auto& c : propName)
        c = to_lower(c);
    return propName.contains("length") && propName.contains("proxy");
};

template <class J, typename T>
concept can_assign = requires(J& js, const T& test) { js[test]; };

template <class J, typename T>
static J& _js_append(J& js, const T& str)
{
    if constexpr (can_assign<J, T>)
        return js[str];
    else
    {
        const auto nullTerminated = str.data()[str.size()] == '\0';
        return nullTerminated ? js[str.data()] : js[typename J::string_t(str.begin(), str.end())];
    }
}

void netvars_storage::request_sort(const netvar_table* table)
{
    const auto idx = std::distance<const netvar_table*>(_begin(data_), table);
    sortRequested_.push_back(idx);
}

void netvars_storage::sort()
{
    const auto sortEnd = std::unique(_begin(sortRequested_), _end(sortRequested_));
    for (const auto idx : range_view(sortRequested_, sortEnd))
        data_[idx].sort();
    sortRequested_.clear();
}

void netvars_storage::add(netvar_table&& table)
{
#ifdef MERGE_DATA_TABLES
    FD_ASSERT(!this->find(table.name()), "Duplicate detected");
#endif
    data_.emplace_back(std::move(table));
}

netvar_table* netvars_storage::add(string&& name, const bool root)
{
#ifdef MERGE_DATA_TABLES
    FD_ASSERT(!this->find(name), "Duplicate detected");
#endif
    return &data_.emplace_back(std::move(name), root);
};

netvar_table* netvars_storage::add(const string_view name, const bool root)
{
#ifdef MERGE_DATA_TABLES
    FD_ASSERT(!this->find(name), "Duplicate detected");
#endif
    return &data_.emplace_back(string(name), root);
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
static auto _find_name(T& rng, const string_view name) -> decltype(rng.data())
{
    for (auto& item : range_view(rng))
    {
        if (item.name() == name)
            return &item;
    }

    return nullptr;
}

netvar_table* netvars_storage::find(const string_view name)
{
    return _find_name(data_, name);
}

const netvar_table* netvars_storage::find(const string_view name) const
{
    return _find_name(data_, name);
}

struct array_info
{
    string_view name;
    size_t      size = 0;
};

static auto _get_props_range(const valve::recv_table* recvTable)
{
    const auto& rawProps = recvTable->props;

    auto front = &rawProps.front();
    auto back  = &rawProps.back();

    if (_is_base_class(front))
        ++front;
    if (_is_length_proxy(back))
        --back;

    return std::pair(front, back + 1);
}

static size_t _get_array_size(const valve::recv_prop* arrayStart, const valve::recv_prop* arrayEnd)
{
    size_t arraySize = 1;
    for (auto& tmp : range_view(arrayStart + 1, arrayEnd))
    {
        if (arrayStart->type != tmp.type)
            break;
        ++arraySize;
    }

    return arraySize;
}

static size_t _get_array_size(const valve::recv_prop* arrayStart, const valve::recv_prop* arrayEnd, const string_view propName)
{
    // todo: try extract size from length proxy

    size_t arraySize = 1;
    for (auto& tmp : range_view(arrayStart + 1, arrayEnd))
    {
        if (arrayStart->type != tmp.type)
            break;
        if (!equal(propName, tmp.name))
            break;
        ++arraySize;
    }

    return arraySize;
}

static size_t _get_array_size(const basic_netvar_info* info)
{
    FD_ASSERT(dynamic_cast<const netvar_info*>(info));
    return static_cast<const netvar_info*>(info)->array_size();
}

static size_t _store(const valve::recv_prop* arrayStart, const string_view propName, const valve::recv_prop* arrayEnd, netvar_table* customTable, size_t extraOffset = 0)
{
    FD_ASSERT(arrayStart->type != valve::DPT_DataTable, "Array DataTable detected!");

    if (!propName.ends_with("[0]"))
    {
        const auto realPropName = propName.substr(0, propName.find('['));
        return _get_array_size(arrayStart, arrayEnd, realPropName);
    }

    const auto realPropName = propName.substr(0, propName.size() - 3);
    FD_ASSERT(!realPropName.contains(']'));

    if (const auto found = customTable->find(realPropName); found != nullptr)
        return _get_array_size(found);

    const auto arraySize = _get_array_size(arrayStart, arrayEnd, realPropName);
    customTable->add(new netvar_info(arrayStart->offset + extraOffset, arrayStart, arraySize, realPropName));

    return arraySize;
}

static void _store(const valve::recv_prop* prop, const string_view propName, netvar_table* customTable, size_t extraOffset = 0)
{
    if (auto found = customTable->find(propName); found != nullptr)
    {
        (void)found;
        return;
    }

    customTable->add(new netvar_info(prop->offset + extraOffset, prop, 0, propName));
}

static size_t _store(const valve::recv_prop* arrayStart, const valve::recv_table* recvTable, const valve::recv_prop* arrayEnd, netvar_table* customTable, size_t extraOffset = 0)
{
    FD_ASSERT(arrayStart->type != valve::DPT_DataTable, "not implemented");

    const string_view tableName(recvTable->name);
    if (const auto found = customTable->find(tableName); found != nullptr)
        return _get_array_size(found);

    const auto arraySize = _get_array_size(arrayStart, arrayEnd);
    customTable->add(new netvar_info(arrayStart->offset + extraOffset, arrayStart, arraySize, tableName));
    return arraySize;
}

// merge data tables
static void _parse(const valve::recv_table* recvTable, netvar_table* customTable, size_t rootOffset = 0)
{
    auto [propsBegin, propsEnd] = _get_props_range(recvTable);

    if (propsBegin == propsEnd)
        return;

    if (propsBegin->name[0] == '0')
        propsBegin += _store(propsBegin, recvTable, propsEnd, customTable, rootOffset);

    for (auto prop = propsBegin; prop != propsEnd; ++prop)
    {
        const string_view propName(prop->name);
        if (_can_skip_netvar(propName))
            continue;
        if (prop->type != valve::DPT_DataTable)
        {
            if (propName.ends_with(']'))
                prop += _store(prop, propName, propsEnd, customTable, rootOffset) - 1;
            else
                _store(prop, propName, customTable, rootOffset);
        }
        else if (const auto dt = prop->data_table; dt && !dt->props.empty())
        {
            _parse(dt, customTable, rootOffset + prop->offset);
        }
    }
}

static string_view _correct_recv_name(const string_view name)
{
    // reserved
    return name;
}

static string_view _correct_recv_name(const char* name)
{
    return _correct_recv_name(string_view(name));
}

[[maybe_unused]] static void _correct_recv_name(const string& name)
{
    (void)name;
}

// store all data tables
static void _parse(const valve::recv_table* recvTable, netvars_storage* storage)
{
    FD_ASSERT_PANIC("WIP");

    /*const auto [propsBegin, propsEnd] = _get_props_range(recvTable);
    if (std::distance(propsBegin, propsEnd) == 0)
        return;
    const auto tableName = _correct_recv_name(recvTable->name);
    if (storage->find(tableName))
        return;
    const auto customTable = storage->add(tableName, recvTable->in_main_list);
    for (auto prop = propsBegin; prop != propsEnd; ++prop)
    {
        const string_view propName(prop->name);
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

void netvars_storage::iterate_client_class(const valve::client_class* rootClass, const string_view debugName)
{
    FD_ASSERT(data_.empty(), "Iterate recv tables first!");

    for (auto& clientClass : range_view(rootClass))
    {
        if (!clientClass.table)
            continue;
        if (clientClass.table->props.empty())
            continue;
#ifdef MERGE_DATA_TABLES
        netvar_table tmp(_correct_class_name(clientClass.name), true);
        _parse(clientClass.table, &tmp);
        if (!_empty(tmp))
            this->add(std::move(tmp));
#else
        _parse(clientClass.table, this);
#endif
    }

    if (log_active())
    {
        const auto name = debugName.empty() ? rootClass->name : debugName;
        log_unsafe(make_string("netvars - ", name, " data tables stored (", format_int(data_.size()), ')'));
    }
}

struct data_map_parse_result
{
    size_t created = 0;
    size_t updated = 0;
};

static auto _parse(const valve::data_map* rootMap, netvars_storage* storage)
{
    data_map_parse_result result;
    for (auto& map : range_view(rootMap))
    {
        if (map.data.empty())
            continue;
        string className;
#ifdef MERGE_DATA_TABLES
        className = _correct_class_name(map.name);
#else
        const auto nameBegin = static_cast<const char*>(memchr(map.name, '_', std::numeric_limits<size_t>::max()));
        write_string(className, "DT_", nameBegin);
        _correct_recv_name(className);
#endif
        auto       table      = storage->find(className);
        const auto tableAdded = table;
        if (!table)
            table = storage->add(std::move(className), true);
        else
            storage->request_sort(table);

        const auto sizeBefore = _size(*table);
        for (auto& desc : range_view(map.data))
        {
            if (desc.type == valve::FIELD_EMBEDDED)
            {
                // not implemented
                if (desc.description)
                    FD_ASSERT_PANIC("Embedded datamap detected");
            }
            else if (desc.name)
            {
                if (_can_skip_netvar(desc.name))
                    continue;
                const string_view name(desc.name);
                if (!tableAdded && table->find(name)) // todo: check & correct type
                    continue;
                // fieldOffset[TD_OFFSET_NORMAL], array replaced with two varaibles
                table->add(new netvar_info(static_cast<size_t>(desc.offset), &desc, 0, name));
            }
        }

        const auto tableUpdated = sizeBefore != _size(*table);

        if (tableAdded)
            ++result.created;
        else if (tableUpdated)
            ++result.updated;
    }
    return result;
}

void netvars_storage::iterate_datamap(const valve::data_map* rootMap, const string_view debugName)
{
    FD_ASSERT(!data_.empty(), "Iterate datamap after recv tables!");

    const auto result = _parse(rootMap, this);

    if (log_active())
    {
        // clang-format off
        log_unsafe(make_string(
            "netvars - ", debugName.empty() ? rootMap->name : debugName,
            " data maps stored (", format_int(result.created), "), ",
            "updated (", format_int(result.updated), ')'
        ));
        // clang-format on
    }
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

    invoke(logger, "netvars - handmade netvars stored");
}
#endif

//------

static auto _drop_default_path(wstring_view buff, const wstring_view prefix)
{
    if (buff.starts_with(prefix))
    {
        auto       prefixSize = prefix.size();
        const auto chr        = buff[prefixSize];
        if (chr == '\\' || chr == '/')
            ++prefixSize;
        buff.remove_prefix(prefixSize);
    }

    return buff;
}

void netvars_storage::log_netvars(netvars_log& data)
{
    FD_ASSERT(!data.dir.empty());
    FD_ASSERT(!data.file.name.empty());
    FD_ASSERT(!data.file.extension.empty());

    this->sort();

    json_unsorted jsRoot;

    for (const auto& table : range_view(data_))
    {
        FD_ASSERT(!_empty(table));

        auto& buff = _js_append(jsRoot, table.name());
        for (const auto info : table)
        {
            using str_t = json_unsorted::string_t;

            const auto name   = info->name();
            const auto type   = info->type();
            const auto offset = info->offset();

            if (type.empty())
            {
                buff.push_back({
                    {"name",    str_t(name)},
                    { "offset", offset     }
                });
            }
            else
            {
                buff.push_back({
                    {"name",    str_t(name)},
                    { "offset", offset     },
                    { "type",   str_t(type)}
                });
            }
        }
    }

    namespace jd = nlohmann::detail; // NOLINT(misc-unused-alias-decls)
    data.buff.reserve(1024 * 210);   // filse size ~210 kb
    jd::serializer<json_unsorted>(jd::output_adapter(data.buff), data.filler).dump(jsRoot, data.indent > 0, false, data.indent);
    data.buff.shrink_to_fit();

    if (log_active())
        log_unsafe(make_string(L"netvars - logs will be written to ", _drop_default_path(data.dir, netvars_log().dir), data.file.name, data.file.extension));
}

struct generate_info
{
    string_view name;
    string      typeOut;
    string      typeCast;

    generate_info(const basic_netvar_info* info)
    {
        const auto typeRaw = info->type();
        if (typeRaw.empty())
            return;

        const auto isPointer   = typeRaw.ends_with('*');
        const auto typeDecayed = isPointer ? typeRaw.substr(0, typeRaw.size() - 1) : typeRaw;

        write_string(typeOut, typeDecayed, isPointer ? '*' : '&');
        write_string(typeCast, isPointer ? "" : "*", "reinterpret_cast<", typeDecayed, "*>");
        name = info->name();
    }
};

void netvars_storage::generate_classes(netvars_classes& data)
{
    FD_ASSERT(!data.dir.empty());

    this->sort();

#ifdef GENERATE_STRUCT_MEMBERS
#error "not implemented"
#else

    data.files.resize(data_.size() * 2);
    auto file = _begin(data.files);

    std::vector<char>          source;
    std::vector<char>          header;
    std::vector<generate_info> table;

    for (const auto& rawTable : range_view(data_))
    {
        FD_ASSERT(!_empty(rawTable));

        table.clear();
        for (auto i : rawTable)
        {
            if (!i->type().empty())
                table.emplace_back(i);
        }
#ifdef _DEBUG
        const range_view tableDecayed = table;
#define table tableDecayed
#endif

        const auto className = rawTable.name();

        constexpr string_view _formatOff = "// clang-format off\n"
                                           "// ReSharper disable All\n\n";
        constexpr string_view _assertGap = "#ifndef FD_ASSERT\n"
                                           "#define FD_ASSERT(...) (void)0\n"
                                           "#endif\n\n";

        source.clear();
        {
            const auto offsetsClass = make_string(className, "_offsets");

            write_string(
                source,
                _formatOff,
                _assertGap, // STRUCT START
                "static struct\n{\n"
            );
            for (auto& i : table)
            {
                write_string(
                    source, //
                    format("\tsize_t {} = -1;\n", i.name)
                );
            };
            write_string(
                source, // INIT START
                "\n\tvoid init()\n"
                "\t{\n"
            );
            for (auto& i : table)
            {
                write_string(
                    source, //
                    format("\t\tFD_ASSERT({} == -1, \"already set!\";\n", i.name),
                    format("\t\t{} = Netvars->get_offset({}, {});\n", i.name, className, i.name)
                );
            }
            write_string(
                source, // INIT END
                "\t}\n"
            );
            write_string(
                source, // STRUCT END
                "} ",
                format("{};\n\n", offsetsClass)
            );
            for (auto& i : table)
            {
                write_string(
                    source, //
                    format("{} {}::{}()\n", i.typeOut, className, i.name),
                    "{\n",
                    format("\tFD_ASSERT({}.{} != -1, \"not set!\");\n", offsetsClass, i.name),
                    format("\tconst auto addr = reinterpret_cast<uintptr_t>(this) + {}.{};\n", offsetsClass, i.name),
                    format("\treturn {}(addr);\n", i.typeCast),
                    "}\n"
                );
            };
        }

        header.clear();
        {
            write_string(
                header, //
                _formatOff
            );
            for (auto& i : table)
            {
                write_string(
                    header, //
                    format("{} {}():\n", i.typeOut, i.name)
                );
            };
        }

        const auto storeFile = [&](const auto extension, const auto& buff) {
            write_string(file->name, className, extension);
            write_string(file->data, buff);
            ++file;
        };

        storeFile("_h", header);
        storeFile("_cpp", source);

        // todo: if !MERGE_DATA_TABLES include wanted files!
    }

#undef table

#endif

    if (log_active())
    {
        // vector to allocate exact memory size
        std::vector<wchar_t> buff;
        // clang-format off
        write_string(buff,
            "netvars - ", format_int(data.files.size()), " classes ",
            "written to ", _drop_default_path(data.dir, netvars_classes().dir)
        );
        // clang-format on
        log_unsafe({ buff.begin(), buff.end() });
    }
}

//-------

#if 0
void netvars_storage::init_default()
{
    // run default initialization

    this->iterate_client_class(FD_OBJECT_GET(base_client)->GetAllClasses(), "All");

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

    if (log_active())
        log_unsafe(make_string("netvars - ", format_int(data_.size()), " classes stored"));
}

size_t netvars_storage::get_offset(const string_view className, const string_view name) const
{
    FD_ASSERT(sortRequested_.capacity() == 0, "finish not called!");
    const auto offset = this->find(className)->find(name)->offset();
    if (log_active())
        log_unsafe(make_string("netvars - ", className, "->", name, " loaded"));
    return offset;
}

void netvars_storage::clear()
{
    FD_ASSERT(sortRequested_.capacity() == 0, "finish not called!");
    data_.clear();
    data_.shrink_to_fit();
}

basic_netvars_storage* Netvars;
}