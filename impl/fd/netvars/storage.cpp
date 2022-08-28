module;

#include <fd/assert.h>
#include <fd/utility.h>

#include <iomanip>
#include <sstream>
#include <vector>

module fd.netvars.storage;
import fd.netvars.table;
import fd.netvars.type_resolve;
import fd.ctype;
import fd.string.make;
import fd.functional.fn;
import fd.json;
import fd.format;

using namespace fd;
using namespace valve;

using hashed_string_view = string_view;
using hashed_string      = string;

logs_data::logs_data()
{
#ifdef FD_ROOT_DIR
    dir = FD_CONCAT(L"", FD_STRINGIZE(FD_ROOT_DIR), "/.dumps/netvars");
#endif
    file.extension = L".json";
    indent         = 4;
    filler         = ' ';
}

classes_data::classes_data()
{
#ifdef FD_WORK_DIR
    dir = FD_CONCAT(L"", FD_STRINGIZE(FD_WORK_DIR), "/valve_custom");
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

//#define MERGE_DATA_TABLES

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

            const auto lstr = to_lower(prop->name);
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
#ifdef MERGE_DATA_TABLES
    datatable_parser(netvar_table* root_table, const size_t offset = 0)
        : offset(offset)
        , nvtable(root_table)
    {
    }
#else
    datatable_parser(const store_fn& callback, netvar_table* current_table = nullptr)
        : store_data_table(callback)
        , nvtable(current_table)
    {
    }
#endif

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

template <class J>
concept json_support_string_view = requires(J& js, const string_view test)
{
    js[test];
};

template <class J>
static auto& _Json_append(J& js, const string_view str)
{
    if constexpr (json_support_string_view<J>)
        return js[str];
    else
        return js[/* string(str) */ str.data()];
}

class storage_impl : public storage
{
    std::vector<netvar_table> data_;

    netvar_table* find(const hashed_string_view name)
    {
        for (auto& t : data_)
        {
            if (t.name() == name)
                return &t;
        }
        return nullptr;
    }

    const netvar_table* find(const hashed_string_view name) const
    {
        return const_cast<storage_impl*>(this)->find(name);
    }

    template <class S>
    auto add(S&& name)
    {
#ifdef MERGE_DATA_TABLES
        FD_ASSERT(!find(name), "Duplicate detected");
#endif
        return &data_.emplace_back(hashed_string(std::forward<S>(name)));
    };

  public:
    void iterate_client_class(const client_class* root_class) override
    {
        for (auto cclass = root_class; cclass != nullptr; cclass = cclass->next)
        {
            auto rtable = cclass->table;
            if (!rtable || rtable->props.empty())
                continue;

            const datatable_parser parser(
#ifdef MERGE_DATA_TABLES
                this->add(_Correct_class_name(client_class->name))
#else
                [&](const char* table_name) -> netvar_table* {
                    const hashed_string_view name1 = table_name;
                    return this->find(name1) ? nullptr : this->add(name1);
                }
#endif
            );

            invoke(parser, rtable);
        }
    }

    void iterate_datamap(const data_map* root_map) override
    {
        for (auto map = root_map; map != nullptr; map = map->base)
        {
            if (map->data.empty())
                continue;

#ifdef MERGE_DATA_TABLES
            hashed_string class_name = _Correct_class_name(map->name);
#else
            const string_view class_name0 = map->name;
            const auto class_name1        = class_name0.substr(class_name0.find('_'));
            hashed_string class_name      = make_string("DT_", class_name1);
#endif
            auto table = this->find(class_name);
            if (!table)
                table = this->add(std::move(class_name));

            for (auto& desc : map->data)
            {
                if (desc.type == FIELD_EMBEDDED)
                {
                    if (desc.description != nullptr)
                        FD_ASSERT("Embedded datamap detected");
                }
                else if (desc.name != nullptr)
                {
                    const string_view name = desc.name;
                    if (_Can_skip_netvar(name))
                        continue;
                    if (table->find(name)) // todo: check & correct type
                        continue;

                    // fieldOffset[TD_OFFSET_NORMAL], array replaced with two varaibles
                    table->add(static_cast<size_t>(desc.offset), &desc, 0, name);
                }
            }
        }
    }

    void store_handmade_netvars() override
    {
#if 0
        const auto baseent = this->find("C_BaseEntity");
        baseent->add<var_map>(0x24, "m_InterpVarMap");
        baseent->add<matrix3x4>(SIG(client, "8B 55 ? 85 D2 74 23 8B 87 ? ? ? ? 8B 4D ? 3B C8", plus(9), deref<1>(), minus(8)), "m_BonesCache", type_utlvector);

        const auto baseanim = this->find("C_BaseAnimating");
        // m_vecRagdollVelocity - 128
        baseanim->add<animation_layer>(SIG(client, "8B 87 ? ? ? ? 83 79 04 00 8B", plus(2), deref<1>()), "m_AnimOverlays", type_utlvector);
        // m_hLightingOrigin - 32
        baseanim->add<float>(SIG(client, "C7 87 ? ? ? ? ? ? ? ? 89 87 ? ? ? ? 8B 8F", plus(2), deref<1>()), "m_flLastBoneSetupTime");
        // ForceBone + 4
        baseanim->add<int>(SIG(client, "89 87 ? ? ? ? 8B 8F ? ? ? ? 85 C9 74 10", plus(2), deref<1>()), "m_iMostRecentModelBoneCounter");
#endif
    }

    void log_netvars(logs_data& data) override
    {
        json_unsorted j_root;

        for (const auto& table : data_)
        {
            if (table.empty())
                continue;

            auto& j_table = _Json_append(j_root, table.name());

            for (const auto& info : table)
            {
                string name(info->name());
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
                        {"name",    std::move(name)},
                        { "offset", offset         },
                        { "type",   string(type)   }
                    });
                }

                /*
                json_unsorted entry;
                _Json_append(entry, "name")   = string(name);
                _Json_append(entry, "offset") = offset;
                if (!type.empty())
                    _Json_append(entry, "type") = string(type);

                j_table.push_back(std::move(entry)); */
            }
        }

        data.buff << std::setw(data.indent) << std::setfill(data.filler) << j_root;
    }

    void generate_classes(classes_data& data) override
    {
        data.files.reserve(data_.size() * 2);

        for (const auto& table : data_)
        {
            if (table.empty())
                continue;

            using file_info = classes_data::file_info;

            file_info h_info, cpp_info;
            auto& h   = h_info.buff;
            auto& cpp = cpp_info.buff;

            const string_view class_name = table.name();

            for (const auto& info : table)
            {
                auto netvar_type = info->type();
                if (netvar_type.empty())
                    continue;

                char ret_char;
                if (netvar_type.ends_with('*')) // type have pointer
                {
                    netvar_type.remove_suffix(1);
                    ret_char = '*';
                }
                else
                {
                    ret_char = '&';
                }

                const string_view netvar_name = info->name();

                //----

                constexpr string_view get_netvar_offset_fn = "FD_OBJECT_GET(basic_netvars_storage)->get_netvar_offset";
                constexpr string_view empty_string         = "{}";

                h << netvar_type << ret_char << ' ' << netvar_name << "();";
                cpp << netvar_type << ret_char << ' ' << class_name << "::" << netvar_name << "()\n";
                cpp << "{\t";
                cpp << "static const auto offset = " << get_netvar_offset_fn << '('
#ifdef MERGE_DATA_TABLES
                    << class_name << ", " << empty_string
#else
                    << empty_string << ", " << class_name
#endif
                    << ", " << netvar_name << ");\n\t";
                cpp << "const auto addr = reinterpret_cast<uintptr_t>(this) + offset;";
                cpp << "return ";
                if (ret_char == '&')
                    cpp << '*';
                cpp << "reinterpret_cast<" << netvar_type << "*>(addr);";
                cpp << "}\n\n";
            }

            h_info.name   = make_string(std::in_place_type<wchar_t>, class_name, "_h");
            cpp_info.name = make_string(std::in_place_type<wchar_t>, class_name, "_cpp");

            data.files.push_back(std::move(h_info));
            data.files.push_back(std::move(cpp_info));
        }
    }

    size_t get_netvar_offset(const string_view class_name, const string_view table_name, const string_view name) const override
    {
#ifdef MERGE_DATA_TABLES
        FD_ASSERT(table_name.empty());
        const auto table = this->find(class_name);
#else
        FD_ASSERT(class_name.empty());
        const auto table = this->find(table_name);
#endif
        return table->find(name)->offset();
    }
};
