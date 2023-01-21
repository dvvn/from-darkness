#include <fd/assert.h>
#include <fd/filesystem.h>
#include <fd/format.h>
#include <fd/functional.h>
#include <fd/json.h>
#include <fd/logger.h>
#include <fd/netvar_storage_impl.h>
#include <fd/netvar_type_resolve.h>
#include <fd/string_info.h>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>

#if 0
namespace std::filesystem
{
    static_assert(sizeof(path) == sizeof(fd::wstring));

    bool create_directory(const fd::wstring& dir)
    {
        return create_directory(reinterpret_cast<const path&>(dir));
    }

    bool is_empty(const fd::wstring& dir)
    {
        return is_empty(reinterpret_cast<const path&>(dir));
    }

    constexpr auto preferred_separator               = path::preferred_separator;
    constexpr path::value_type unpreferred_separator = preferred_separator == '\\' ? '/' : '\\';
} // namespace std::filesystem

namespace fs = std::filesystem;
#endif

using namespace fd;
using namespace valve;

template <typename T>
static bool _file_already_written(const T& fullPath, const std::span<const char> buffer)
{
    std::ifstream fileStored;
    fileStored.rdbuf()->pubsetbuf(nullptr, 0); // disable buffering
    fileStored.open(fullPath, std::ios::binary | std::ios::ate);

    if (!fileStored)
        return false;

    const auto size = fileStored.tellg();
    if (size != buffer.size())
        return false;

#if 0
	const unique_ptr<char[]> buff = new char[size];
	if (!file_stored.read(buff.get(), size))
		return false;
	return std::memcmp(buff.get(), buffer.data(), size) == 0;
#else
    using it_t = std::istream_iterator<char>;
    return std::equal<it_t>(fileStored, {}, buffer.begin());
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

static void _write_to_file(const wstring_view path, const std::span<const char> buff)
{
    // std::ofstream(full_path).write(buff.data(), buff.size());
    FILE* f;
    _wfopen_s(&f, path.data(), L"w");
    _fwrite_nolock(buff.data(), 1, buff.size(), f);
    _fclose_nolock(f);
}

netvars_log::netvars_log()
{
#if defined(FD_ROOT_DIR) && !defined(__RESHARPER__)
    dir = FD_CONCAT_EX(L"", FD_STRINGIZE(FD_ROOT_DIR), "/.dumps/netvars/");
#endif
    // write_string(file.name, engine->GetProductVersionString());
    file.extension = L".json";
    indent         = 4;
    filler         = ' ';
}

netvars_log::~netvars_log()
{
    if (dir.empty())
        return;
    //_Correct_path(dir);
    if (!fs::Directory.create(dir, false))
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
        wstring               path;
        std::span<const char> buff;
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

    const auto dirIsEmpty = fs::Directory.create(dir, false) || fs::Directory.empty(dir);
    if (dirIsEmpty)
        std::ranges::for_each(files, writeBuffer, buildPath);
    else
        std::ranges::for_each(files | std::views::transform(buildPath) | std::views::filter(skipWritten), writeBuffer);
}

netvars_classes::netvars_classes()
{
#if defined(FD_WORK_DIR) && !defined(__RESHARPER__)
    dir = FD_CONCAT_EX(L"", FD_STRINGIZE(FD_WORK_DIR), "/valve_custom/");
#endif
}

//----

[[maybe_unused]] static string _correct_class_name(const string_view name)
{
    if (name[0] == 'C' && name[1] != '_')
    {
        FD_ASSERT(is_alnum(name[1]));
        // internal csgo classes looks like C_***
        // same classes in shared code look like C***
        return make_string("C_", name.substr(1));
    }
    else
    {
        FD_ASSERT(!name.starts_with("DT_"));
        return { name.begin(), name.end() };
    }
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

// #define MERGE_DATA_TABLES
// #define GENERATE_STRUCT_MEMBERS

static auto _is_base_class(const recv_prop* prop)
{
    constexpr string_view str = "baseclass";
    // return std::memcmp(prop->name, str.data(), str.size()) == 0 && prop->name[str.size()] == '\0';
    return prop->name == str;
};

static auto _is_length_proxy(const recv_prop* prop)
{
    if (prop->array_length_proxy)
        return true;
    const string_view propName(prop->name);
    string            lstr;
    lstr.reserve(propName.size());
    std::ranges::transform(propName, std::back_insert_iterator(lstr), to_lower);
    return lstr.contains("length") && lstr.contains("proxy");
};

class datatable_parser
{
#ifdef MERGE_DATA_TABLES
    size_t offset = 0;
#else
    using store_fn = function<netvar_table*(const char*) const>;
    store_fn storeDataTable_;
#endif
    netvar_table* netvarTable_;

    // ReSharper disable once CppMemberFunctionMayBeStatic
    size_t get_offset(const size_t propOffset) const
    {
        return
#ifdef MERGE_DATA_TABLES
            offset + propOffset
#else
            propOffset
#endif
            ;
    }

    datatable_parser get_next(const char* name, const size_t propOffset) const
    {
        return {
#ifdef MERGE_DATA_TABLES
            netvarTable_, offset + propOffset
#else
            storeDataTable_, invoke(storeDataTable_, name)
#endif
        };
    }

    operator bool() const
    {
        return netvarTable_ != nullptr;
    }

    struct array_info
    {
        string_view name;
        size_t      size = 0;
    };

    array_info parse_array(const string_view firstPropName, const recv_prop* arrayStart, const recv_prop* arrayEnd) const
    {
        if (!firstPropName.ends_with("[0]"))
            return {};

        const auto realPropName = firstPropName.substr(0, firstPropName.size() - 3);
        FD_ASSERT(!realPropName.ends_with(']'));
        if (netvarTable_->find(realPropName)) // todo: debug break for type check!
            return { realPropName, 0 };

        // todo: try extract size from length proxy
        size_t arraySize = 1;

        for (auto prop = arrayStart; prop != arrayEnd; ++prop)
        {
            if (arrayStart->type != prop->type) // todo: check is name still same after this (because previously we store this name without array braces)
                break;
#if 1
            if (realPropName != prop->name)
                break;
#else
            // name.starts_with(real_prop_name)
            if (std::memcmp(prop.name, real_prop_name.data(), real_prop_name.size()) != 0)
                break;
            // name.size() == real_prop_name.size()
            if (prop.name[real_prop_name.size()] != '\0')
                break;
#endif
            ++arraySize;
        }

        return { realPropName, arraySize };
    }

    static auto get_props_range(const recv_table* recvTable)
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

  public:
    datatable_parser
#ifdef MERGE_DATA_TABLES
        (netvar_table* rootTable, const size_t offset = 0)
        : offset(offset)
        , netvarTable_(rootTable)
#else
        (const store_fn& callback, netvar_table* currentTable = nullptr)
        : storeDataTable_(callback)
        , netvarTable_(currentTable)
#endif
    {
    }

    void operator()(const recv_table* dataTable) const
    {
        const auto [props_begin, props_end] = get_props_range(dataTable);
        for (auto itr = props_begin; itr != props_end; ++itr)
        {
            const auto& prop = *itr;
            FD_ASSERT(prop.name != nullptr);
            const string_view propName(prop.name);
            if (_can_skip_netvar(propName))
                continue;

            if (propName.rfind(']') != propName.npos)
            {
                FD_ASSERT(prop.type != DPT_DataTable, "Array DataTable detected!");
                const auto arrayInfo = parse_array(propName, itr + 1, props_end);
                if (arrayInfo.size > 0)
                {
                    netvarTable_->add(get_offset(prop.offset), itr, arrayInfo.size, arrayInfo.name);
                    itr += arrayInfo.size - 1;
                }
            }
            else if (prop.type != DPT_DataTable)
            {
                netvarTable_->add(get_offset(prop.offset), itr, 0, propName);
            }
            else if (prop.data_table && !prop.data_table->props.empty())
            {
                const auto next = get_next(prop.data_table->name, prop.offset);
#ifndef MERGE_DATA_TABLES
                if (!next)
                    continue;
#endif
                invoke(next, prop.data_table);
            }
        }
    }
};

template <class J, typename T>
concept can_assign = requires(J& js, const T& test) { js[test]; };

template <class J, typename T>
static auto _append(J& js, const T& str)
{
    if constexpr (can_assign<J, T>)
        return &js[str];
    else
    {
        const auto nullTerminated = str.data()[str.size()] == '\0';
        return nullTerminated ? &js[str.data()] : &js[typename J::string_t(str.begin(), str.end())];
    }
}

template <typename T>
static void _stream_to(std::ostringstream&& stream, T& to)
{
    if constexpr (std::assignable_from<T, std::string>)
        to = std::move(stream).str();
    else
    {
        const auto str = stream.view();
        to.assign(str.begin(), str.end());
    }
}

template <class T>
static T _stream_to(std::ostringstream&& stream)
{
    T to;
    _stream_to(std::move(stream), to);
    return to;
}

void netvars_storage::request_sort(const netvar_table* table)
{
    const auto idx = std::distance<const netvar_table*>(data_.data(), table);
    sortRequested_.push_back(idx);
}

void netvars_storage::sort()
{
    // ReSharper disable once CppUseRangeAlgorithm
    const auto sortEnd = std::unique(sortRequested_.begin(), sortRequested_.end());
    std::for_each(sortRequested_.begin(), sortEnd, [&](const size_t idx) {
        auto& t = data_[idx];
        std::ranges::sort(t, [](auto& l, auto& r) {
            return l->offset() < r->offset();
        });
    });
    sortRequested_.clear();
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
}

template <class T>
static auto _find_name(T& rng, const string_view name)
{
    const auto found = std::ranges::find(rng, name, &std::remove_const_t<T>::value_type::name);
    return found == rng.end() ? nullptr : &*found;
}

netvar_table* netvars_storage::find(const string_view name)
{
    return _find_name(data_, name);
}

const netvar_table* netvars_storage::find(const string_view name) const
{
    return _find_name(data_, name);
}

static constexpr auto _OverrideName = [](auto* root, auto debugName) -> string_view {
    return debugName.empty() ? root->name : debugName;
};

void netvars_storage::iterate_client_class(const client_class* rootClass, const string_view debugName)
{
    FD_ASSERT(data_.empty(), "Iterate recv tables first!");

    constexpr auto skipBadTable = [](auto& clientClass) {
        return clientClass.table && !clientClass.table->props.empty();
    };

    std::ranges::for_each(client_class_range(rootClass) | std::views::filter(skipBadTable), [this](auto& clientClass) {
        const datatable_parser parser = {
#ifdef MERGE_DATA_TABLES
            this->add(_correct_class_name(clientClass.name))
#else
            [&](const string_view tableName) {
                return this->add(tableName, false);
            }
#endif
        };
        invoke(parser, clientClass.table);
    });

    if (!log_active())
        return;
    log_unsafe(fd::format( //-
        "netvars - {} data tables stored",
        _OverrideName(rootClass, debugName)
    ));
}

void netvars_storage::iterate_datamap(const data_map* rootMap, const string_view debugName)
{
    FD_ASSERT(!data_.empty(), "Iterate datamap after recv tables!");

    for (auto map = rootMap; map != nullptr; map = map->base)
    {
        if (map->data.empty())
            continue;
        auto className =
#ifdef MERGE_DATA_TABLES
            _correct_class_name(map->name);
#else
            [&] {
                const string_view className0(map->name);
                const auto        className1 = className0.substr(className0.find('_'));
                return make_string("DT_", className1);
            }();
#endif
        auto table = this->find(className);
        bool tableAdded;
        if (!table)
        {
            table      = this->add(std::move(className), true);
            tableAdded = true;
        }
        else
        {
            this->request_sort(table);
            tableAdded = false;
        }

        for (auto& desc : map->data)
        {
            if (desc.type == FIELD_EMBEDDED)
            {
                if (desc.description != nullptr)
                    FD_ASSERT_UNREACHABLE("Embedded datamap detected");
            }
            else if (desc.name != nullptr)
            {
                if (_can_skip_netvar(desc.name))
                    continue;
                const string_view name = desc.name;
                if (!tableAdded && table->find(name)) // todo: check & correct type
                    continue;
                // fieldOffset[TD_OFFSET_NORMAL], array replaced with two varaibles
                table->add(static_cast<size_t>(desc.offset), &desc, 0, name);
            }
        }
    }

    // log_unsafe(format("netvars - {} data maps stored", _OverrideName( rootMap, debugName)));
}
#if 0
void netvars_storage::store_handmade_netvars()
{
    const auto baseent = this->find("C_BaseEntity");
    this->request_sort(baseent);
    baseent->add<var_map>(0x24, "m_InterpVarMap");
    baseent->add<matrix3x4>(SIG(client, "8B 55 ? 85 D2 74 23 8B 87 ? ? ? ? 8B 4D ? 3B C8", plus(9), deref<1>(), minus(8)), "m_BonesCache", type_utlvector);

    const auto baseanim = this->find("C_BaseAnimating");
    this->request_sort(baseanim);
    // m_vecRagdollVelocity - 128
    baseanim->add<animation_layer>(SIG(client, "8B 87 ? ? ? ? 83 79 04 00 8B", plus(2), deref<1>()), "m_AnimOverlays", type_utlvector);
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

static constexpr auto _SkipEmpty = [](auto& rng) {
    return !rng.empty();
};

template <class S>
struct simple_netvar_info
{
    S      name;
    size_t offset;
    S      type;

    simple_netvar_info(const basic_netvar_info& info)
    {
        const auto nativeName = info.name();
        const auto nativeType = info.type();

        name   = { nativeName.begin(), nativeName.end() };
        offset = info.offset();
        type   = { nativeType.begin(), nativeType.end() };
    }
};

void netvars_storage::log_netvars(netvars_log& data)
{
    FD_ASSERT(!data.dir.empty());
    FD_ASSERT(!data.file.name.empty());
    FD_ASSERT(!data.file.extension.empty());

    this->sort();

    json_unsorted jsRoot;
    using json_string = json_unsorted::string_t;

    constexpr auto unpackTable = [](auto& info) -> simple_netvar_info<json_string> {
        return *info;
    };

    constexpr auto storeTable = [](auto info, auto* buff) {
        if (info.type.empty())
        {
            buff->push_back({
                {"name",    std::move(info.name)},
                { "offset", info.offset         }
            });
        }
        else
        {
            buff->push_back({
                {"name",    std::move(info.name)},
                { "offset", info.offset         },
                { "type",   std::move(info.type)}
            });
        }
    };

    std::ranges::for_each(data_ | std::views::filter(_SkipEmpty), [&jsRoot](auto& table) {
        std::ranges::for_each(table, bind_back(storeTable, _append(jsRoot, table.name())), unpackTable);
    });

    _stream_to(std::ostringstream() << std::setw(data.indent) << std::setfill(data.filler) << jsRoot, data.buff);

    if (!log_active())
        return;
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

        typeOut  = make_string(typeDecayed, isPointer ? '*' : '&');
        typeCast = make_string(isPointer ? "" : "*", "reinterpret_cast<", typeDecayed, "*>");
        name     = info->name();
    }
};

void netvars_storage::generate_classes(netvars_classes& data)
{
    FD_ASSERT(!data.dir.empty());

    this->sort();

#ifdef GENERATE_STRUCT_MEMBERS
#error "not implemented"
#else
    data.files.reserve(data_.size() * 2);

    constexpr auto reqType = [](auto& i) {
        return !i->type().empty();
    };

    constexpr auto genInfo = [](auto& i) -> generate_info {
        return i.get();
    };

    // reuse the memory
    std::vector<char> source, header;

    std::ranges::for_each(data_ | std::views::filter(_SkipEmpty), [&](auto& rawTable) {
        auto              validTable = rawTable | std::views::filter(reqType) | std::views::transform(genInfo);
        const std::vector table(validTable.begin(), validTable.end());

        const auto className = rawTable.name();

        source.clear();
        {
            const auto offsetsClass = make_string(className, "_offsets");

            // clang-format off
            write_string(source,
                         "// clang-format off\n",
                         "#ifndef FD_ASSERT\n#define FD_ASSERT(...) #endif\n\n", //
                         "static struct\n{\n");
            // clang-format on
            std::ranges::for_each(table, [&](auto& i) {
                write_string(source, "\tsize_t ", i.name, " = -1;\n");
            });
            write_string(source, "\n\tvoid init()\n\t{\n");
            std::ranges::for_each(table, [&](auto& i) {
                // clang-format off
                write_string(source, 
                             "\t\tFD_ASSERT(", i.name, " == -1, already set!);\n",
                             "\t\t", i.name, " = Netvars->get_offset(", className, ", ", i.name, ");\n");
                // clang-format on
            });
            write_string(source, "} ", offsetsClass, ";\n\n");
            std::ranges::for_each(table, [&](auto& i) {
                // clang-format off
                write_string(source,
                             i.typeOut, ' ', className, "::", i.name, "()\n",
                             "{\n",
                             "\tFD_ASSERT(", offsetsClass, '.', i.name, " != -1, not set!);\n",
                             "\tconst auto addr = reinterpret_cast<uintptr_t>(this) + ", offsetsClass, '.', i.name, ";\n",
                             "\treturn ", i.typeCast, "(addr);\n",
                             "}\n");
                // clang-format on
            });
        }

        header.clear();
        {
            std::ranges::for_each(table, [&](auto& i) {
                // clang-format off
                write_string(header,
                             "// clang-format off\n\n",
                              i.typeOut, ' ', i.name, "();\n");
                // clang-format on
            });
        }

        // ReSharper disable once CppInconsistentNaming
        const auto store_file = [&](const string_view extension, const std::span<const char>& buff) {
            auto& [fileName, fileData] = data.files.emplace_back();
            write_string(fileName, className, extension);
            fileData.assign(buff.begin(), buff.end());
        };

        store_file("_h", header);
        store_file("_cpp", source);

        // todo: if !MERGE_DATA_TABLES include wanted files!
    });
#endif

    if (log_active())
    {
        log_unsafe(fd::format(
            L"netvars - {} classes written to {}",
            data.files.size(),
            /* _Correct_path */ _drop_default_path(data.dir, netvars_classes().dir)
        ));
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
        log_unsafe(format("netvars - {} classes stored", data_.size()));
}

size_t netvars_storage::get_offset(const string_view className, const string_view name) const
{
#if 0
    // ideally it is better to use std::call_once instead of static
    static const auto once = [&] {
        auto tthis = const_cast<netvars_storage*>(this);
        if (data_.empty())
            tthis->init_default();
        tthis->finish();
        return true;
    }();
#endif
    FD_ASSERT(sortRequested_.capacity() == 0, "finish not called!");
    return this->find(className)->find(name)->offset();
}

//----------

namespace fd
{
    basic_netvars_storage* Netvars;
}