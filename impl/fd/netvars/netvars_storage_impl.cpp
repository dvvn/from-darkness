module;

#include <fd/assert.h>
#include <fd/utility.h>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>

module fd.netvars.storage.impl;
import fd.netvars.type_resolve;
import fd.string.info;
import fd.string.make;
import fd.functional.fn;
import fd.functional.bind;
import fd.json;
import fd.logger;
import fd.filesystem;
//---
// import fd.valve.base_entity;
// import fd.valve.engine_client;
// import fd.valve.base_client;

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
static bool _File_already_written(const T& full_path, const std::span<const char> buffer)
{
    std::ifstream file_stored;
    file_stored.rdbuf()->pubsetbuf(nullptr, 0); // disable buffering
    file_stored.open(full_path, std::ios::binary | std::ios::ate);

    if (!file_stored)
        return false;

    const auto size = file_stored.tellg();
    if (size != buffer.size())
        return false;

#if 0
	const unique_ptr<char[]> buff = new char[size];
	if (!file_stored.read(buff.get(), size))
		return false;
	return std::memcmp(buff.get(), buffer.data(), size) == 0;
#else
    using it_t = std::istream_iterator<char>;
    return std::equal<it_t>(file_stored, {}, buffer.begin());
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

static void _Write_to_file(const wstring& path, const std::span<const char> buff)
{
    // std::ofstream(full_path).write(buff.data(), buff.size());
    FILE* f;
    _wfopen_s(&f, path.data(), L"w");
    _fwrite_nolock(buff.data(), 1, buff.size(), f);
    _fclose_nolock(f);
}

netvars_log::netvars_log()
{
#ifdef FD_ROOT_DIR
    dir = FD_CONCAT(L"", FD_STRINGIZE(FD_ROOT_DIR), "/.dumps/netvars/");
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
    if (!fs::directory.create(dir))
        return;
    const auto full_path = make_string(dir, file.name, file.extension);
    if (_File_already_written(full_path, buff))
        return;
    _Write_to_file(full_path, buff);
}

netvars_classes::~netvars_classes()
{
    if (dir.empty())
        return;
    //_Correct_path(dir);
    const auto file_is_empty = fs::directory.create(dir) || fs::directory.empty(dir);
    for (const auto& [name, buff] : files)
    {
        const auto current_file_path = make_string(dir, name);
        if (!file_is_empty && _File_already_written(current_file_path, buff))
            continue;
        _Write_to_file(current_file_path, buff);
    }
}

netvars_classes::netvars_classes()
{
#ifdef FD_WORK_DIR
    dir = FD_CONCAT(L"", FD_STRINGIZE(FD_WORK_DIR), "/valve_custom/");
#endif
}

//----

static string _Correct_class_name(const string_view name)
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

static bool _Can_skip_netvar(const string_view name)
{
    return name.contains('.');
}

// #define MERGE_DATA_TABLES
// #define GENERATE_STRUCT_MEMBERS

class datatable_parser
{
#ifdef MERGE_DATA_TABLES
    size_t offset = 0;
#else
    using store_fn = function_view<netvar_table*(const char*) const>;
    store_fn store_data_table;
#endif
    netvar_table* nvtable;

    size_t get_offset(const size_t prop_offset) const
    {
        return
#ifdef MERGE_DATA_TABLES
            offset + prop_offset
#else
            prop_offset
#endif
            ;
    }

    datatable_parser get_next(const char* name, const size_t prop_offset) const
    {
        return {
#ifdef MERGE_DATA_TABLES
            nvtable, offset + prop_offset
#else
            store_data_table, invoke(store_data_table, name)
#endif
        };
    }

    operator bool() const
    {
        return nvtable != nullptr;
    }

    struct array_info
    {
        string_view name;
        size_t size = 0;
    };

    array_info parse_array(const string_view first_prop_name, const recv_prop* array_start, const recv_prop* array_end) const
    {
        if (!first_prop_name.ends_with("[0]"))
            return {};

        const auto real_prop_name = first_prop_name.substr(0, first_prop_name.size() - 3);
        FD_ASSERT(!real_prop_name.ends_with(']'));
        if (nvtable->find(real_prop_name)) // todo: debug break for type check!
            return { real_prop_name, 0 };

        // todo: try extract size from length proxy
        size_t array_size = 1;

        for (auto prop = array_start; prop != array_end; ++prop)
        {
            if (array_start->type != prop->type) // todo: check is name still same after this (because previously we store this name without array braces)
                break;
#if 1
            if (real_prop_name != prop->name)
                break;
#else
            // name.starts_with(real_prop_name)
            if (std::memcmp(prop.name, real_prop_name.data(), real_prop_name.size()) != 0)
                break;
            // name.size() == real_prop_name.size()
            if (prop.name[real_prop_name.size()] != '\0')
                break;
#endif
            ++array_size;
        }

        return { real_prop_name, array_size };
    }

    static auto get_props_range(const recv_table* recv_table)
    {
        constexpr auto is_base_class = [](const recv_prop* prop) {
            constexpr string_view str = "baseclass";
            // return std::memcmp(prop->name, str.data(), str.size()) == 0 && prop->name[str.size()] == '\0';
            return prop->name == str;
        };

        constexpr auto is_length_proxy = [](const recv_prop* prop) {
            if (prop->array_length_proxy)
                return true;
            const auto buff_size = str_len(prop->name);
            string lstr;
            lstr.reserve(buff_size);
            std::transform(prop->name, prop->name + buff_size, std::back_insert_iterator(lstr), to_lower);
            return lstr.contains("length") && lstr.contains("proxy");
        };

        const auto& raw_props = recv_table->props;

        auto front = &raw_props.front();
        auto back  = &raw_props.back();

        if (is_base_class(front))
            ++front;
        if (is_length_proxy(back))
            --back;

        return std::pair(front, back + 1);
    }

  public:
    datatable_parser
#ifdef MERGE_DATA_TABLES
        (netvar_table* root_table, const size_t offset = 0)
        : offset(offset)
        , nvtable(root_table)
#else
        (const store_fn& callback, netvar_table* current_table = nullptr)
        : store_data_table(callback)
        , nvtable(current_table)
#endif
    {
    }

    void operator()(const recv_table* dtable) const
    {
        const auto [props_begin, props_end] = get_props_range(dtable);
        for (auto itr = props_begin; itr != props_end; ++itr)
        {
            const auto& prop = *itr;
            FD_ASSERT(prop.name != nullptr);
            const string_view prop_name = prop.name;
            if (_Can_skip_netvar(prop_name))
                continue;

            if (prop_name.rfind(']') != prop_name.npos)
            {
                FD_ASSERT(prop.type != DPT_DataTable, "Array DataTable detected!");
                const auto array_info = parse_array(prop_name, itr + 1, props_end);
                if (array_info.size > 0)
                {
                    nvtable->add(get_offset(prop.offset), itr, array_info.size, array_info.name);
                    itr += array_info.size - 1;
                }
            }
            else if (prop.type != DPT_DataTable)
            {
                nvtable->add(get_offset(prop.offset), itr, 0, prop_name);
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
concept can_append = requires(J& js, const T& test) { js[test]; };

template <class J, typename T>
static auto& _Append(J& js, const T& str)
{
    if constexpr (can_append<J, T>)
        return js[str];
    else
    {
        const auto null_terminated = str.data()[str.size()] == '\0';
        return null_terminated ? js[str.data()] : js[typename J::string_t(str.begin(), str.end())];
    }
}

template <typename T>
static void _Stream_to(std::ostringstream&& stream, T& to)
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
static T _Stream_to(std::ostringstream&& stream)
{
    T to;
    _Stream_to(std::move(stream), to);
    return to;
}

void netvars_storage::request_sort(const netvar_table* table)
{
    const auto idx = std::distance<const netvar_table*>(data_.data(), table);
    sort_requested_.push_back(idx);
}

void netvars_storage::sort()
{
    const auto sort_end = std::unique(sort_requested_.begin(), sort_requested_.end());
    std::for_each(sort_requested_.begin(), sort_end, [&](const size_t idx) {
        auto& t = data_[idx];
        std::sort(t.begin(), t.end(), [](auto& l, auto& r) {
            return l->offset() < r->offset();
        });
    });
    sort_requested_.clear();
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
    sort_requested_.reserve(1);
#endif
}

template <class T>
static auto _Find_name(T& rng, const string_view name)
{
    const auto bg    = rng.begin();
    const auto ed    = rng.end();
    const auto found = std::find_if(bg, ed, [=](auto& item) {
        return item.name() == name;
    });
    return found == ed ? nullptr : &*found;
}

netvar_table* netvars_storage::find(const string_view name)
{
    return _Find_name(data_, name);
}

const netvar_table* netvars_storage::find(const string_view name) const
{
    return _Find_name(data_, name);
}

constexpr auto _Override_name = [](auto* root, auto debug_name) -> string_view {
    return debug_name.empty() ? root->name : debug_name;
};

void netvars_storage::iterate_client_class(const client_class* root_class, const string_view debug_name)
{
    FD_ASSERT(data_.empty(), "Iterate recv tables first!");

    for (auto cclass = root_class; cclass != nullptr; cclass = cclass->next)
    {
        auto rtable = cclass->table;
        if (!rtable || rtable->props.empty())
            continue;

        const datatable_parser parser = {
#ifdef MERGE_DATA_TABLES
            this->add(_Correct_class_name(client_class->name))
#else
            [&](const string_view table_name) {
                return this->add(table_name, false);
            }
#endif
        };
        invoke(parser, rtable);
    }

    invoke(logger, "netvars - {} data tables stored", bind_front(_Override_name, root_class, debug_name));
}

void netvars_storage::iterate_datamap(const data_map* root_map, const string_view debug_name)
{
    FD_ASSERT(!data_.empty(), "Iterate datamap after recv tables!");

    for (auto map = root_map; map != nullptr; map = map->base)
    {
        if (map->data.empty())
            continue;
        string class_name =
#ifdef MERGE_DATA_TABLES
            _Correct_class_name(map->name);
#else
            [&] {
                const string_view class_name0 = map->name;
                const auto class_name1        = class_name0.substr(class_name0.find('_'));
                return make_string("DT_", class_name1);
            }();
#endif
        auto table = this->find(class_name);
        bool table_added;
        if (!table)
        {
            table       = this->add(std::move(class_name), true);
            table_added = true;
        }
        else
        {
            this->request_sort(table);
            table_added = false;
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
                if (_Can_skip_netvar(desc.name))
                    continue;
                const string_view name = desc.name;
                if (!table_added && table->find(name)) // todo: check & correct type
                    continue;
                // fieldOffset[TD_OFFSET_NORMAL], array replaced with two varaibles
                table->add(static_cast<size_t>(desc.offset), &desc, 0, name);
            }
        }
    }

    invoke(logger, "netvars - {} data maps stored", bind_front(_Override_name, root_map, debug_name));
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

static auto _Drop_default_path(wstring_view buff, const wstring_view prefix)
{
    if (buff.starts_with(prefix))
    {
        auto prefix_size = prefix.size();
        const auto chr   = buff[prefix_size];
        if (chr == '\\' || chr == '/')
            ++prefix_size;
        buff.remove_prefix(prefix_size);
    }

    return buff;
}

void netvars_storage::log_netvars(netvars_log& data)
{
    FD_ASSERT(!data.dir.empty());
    FD_ASSERT(!data.file.name.empty());
    FD_ASSERT(!data.file.extension.empty());

    this->sort();

    json_unsorted j_root;
    using json_string = typename json_unsorted::string_t;

    for (const auto& table : data_)
    {
        if (table.empty())
            continue;

        auto& j_table = _Append(j_root, table.name());

        for (const auto& info : table)
        {
            json_string name(info->name());
            const auto offset      = info->offset();
            const string_view type = info->type();

            if (type.empty())
            {
                j_table.push_back({
                    {"name",    std::move(name)},
                    { "offset", offset         }
                });
            }
            else
            {
                j_table.push_back({
                    {"name",    std::move(name)  },
                    { "offset", offset           },
                    { "type",   json_string(type)}
                });
            }
        }
    }

    std::ostringstream tmp_buff;
    tmp_buff << std::setw(data.indent) << std::setfill(data.filler) << j_root;
    _Stream_to(std::move(tmp_buff), data.buff);

    invoke(logger, "netvars - logs will be written to {}", [&] {
        return make_string(/* _Correct_path */ (_Drop_default_path(data.dir, netvars_log().dir)), data.file.name, data.file.extension);
    });
}

void netvars_storage::generate_classes(netvars_classes& data)
{
    FD_ASSERT(!data.dir.empty());

    this->sort();

#ifdef GENERATE_STRUCT_MEMBERS
#error "not implemented"
#else
    data.files.reserve(data_.size() * 2);

    for (const auto& table : data_)
    {
        if (table.empty())
            continue;

        std::ostringstream h, cpp;

        const string_view class_name = table.name();

        for (const auto& info : table)
        {
            auto netvar_type_raw = info->type();
            if (netvar_type_raw.empty())
                continue;

            const auto netvar_type_pointer = netvar_type_raw.ends_with('*');

            const auto netvar_type_decayed = netvar_type_pointer ? netvar_type_raw.substr(0, netvar_type_raw.size() - 1) : netvar_type_raw;
            const auto netvar_type_out     = make_string(netvar_type_decayed, netvar_type_pointer ? '*' : '&');
            const auto netvar_type_cast    = make_string(netvar_type_pointer ? "" : "*", "reinterpret_cast<", netvar_type_decayed, "*>");

            const string_view netvar_name = info->name();

            constexpr string_view offset_getter_fn = "netvars_storage->get_offset";
            constexpr string_view args_separator   = ", ";

            h << netvar_type_out << ' ' << netvar_name << "();\n";
            cpp << netvar_type_out << ' ' << class_name << "::" << netvar_name << "()\n"
                << '{'
                /*.......*/
                << "\tstatic const auto offset = " << offset_getter_fn << '(' << class_name << ", " << netvar_name << ");\n"
                << "\tconst auto addr = reinterpret_cast<uintptr_t>(this) + offset;\n"
                << "\treturn " << netvar_type_cast << "(addr);\n"
                << "}\n\n";
        }

        const auto store_file = [&](const auto extension, std::ostringstream& buff) {
            auto& [file_name, file_data] = data.files.emplace_back();
            write_string(file_name, class_name, extension);
            _Stream_to(std::move(buff), file_data);
        };

        store_file("_h", h);
        store_file("_cpp", cpp);

        // todo: if !MERGE_DATA_TABLES include wanted files!
    }
#endif

    invoke(logger, "netvars - {} classes written to {}", data.files.size(), [&] {
        return /* _Correct_path */ (_Drop_default_path(data.dir, netvars_classes().dir));
    });
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
    sort_requested_.clear();
    sort_requested_.shrink_to_fit();

    invoke(logger, "netvars - {} classes stored", data_.size());
}

size_t netvars_storage::get_offset(const string_view class_name, const string_view name) const
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
    FD_ASSERT(sort_requested_.capacity() == 0, "finish not called!");
    return this->find(class_name)->find(name)->offset();
}
