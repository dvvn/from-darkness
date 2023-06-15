#include "core.h"
#include "netvar_storage.h"
#include "tool/functional.h"
#include "valve/client.h"
#include "valve/data_map.h"
#include "valve/recv_table.h"

#include <fmt/format.h>

#include <algorithm>

namespace fd
{
#ifdef _DEBUG
template <size_t S>
static constexpr size_t strlen(char const (&)[S])
{
    return S - 1;
}
#endif

netvar_info::netvar_info(netvar_source source, basic_netvar_type_cache *type_cache)
    : source_(source)
{
    if (type_cache)
        type_ = type_cache;
}

string_view netvar_info::raw_name() const
{
    return visit(overload(&valve::recv_prop::name, &valve::data_map_description::name), source_);
}

string_view netvar_info::name() const
{
    return visit(
        overload(
            [](valve::recv_prop *prop) -> auto & {
                if (!prop->inside_array)
                    return prop->name;
#if 1
                return std::prev(prop)->name;
#else
                using traits = string_view::traits_type;
                return {prop->name, traits::find(prop->name, -1, '[')};
#endif
            },
            &valve::data_map_description::name),
        source_);
}

static bool is_array(valve::recv_prop *prop)
{
    using pt = valve::recv_prop_type;
    return prop->type == pt::DPT_Array;
}

class prefix_string
{
    static constexpr string_view internal_prefix = "m_";
    string_view type_;

  public:
    prefix_string(const char *type)
        : type_(type)
    {
    }

    prefix_string(string_view type)
        : type_(type)
    {
    }

    operator string_view() const
    {
        return type_;
    }

    bool contains(size_t length) const
    {
        assert(operator bool());

        auto tmp = type_.substr(internal_prefix.length());

        if (tmp.length() <= length)
            return false;
        if (!std::isupper(tmp[length]))
            return false;

        switch (length)
        {
        case 0:
            std::unreachable();
        case 1:
            return std::isupper(tmp.back()) && std::islower(tmp.front());
        default:
            return std::isupper(tmp[length]) && std::ranges::all_of(tmp.substr(0, length), islower);
        }
    }

    bool contains(string_view prefix) const
    {
        assert(operator bool());

        auto tmp = type_.substr(internal_prefix.length());
        if (tmp.length() <= prefix.length() + 1)
            return false;
        if (!std::isupper(tmp[prefix.length()]))
            return false;

        return tmp.starts_with(prefix);
    }

    explicit operator bool() const
    {
        // m_xX
        if (type_.size() < internal_prefix.size() + 2)
            return false;
        if (!type_.starts_with(internal_prefix))
            return false;
        return true;
    }

    string_view extract(size_t max_length = std::numeric_limits<size_t>::max() - 1) const
    {
        assert(operator bool());

        auto start = type_.data() + internal_prefix.size();
        auto end   = type_.data() + std::min(type_.size(), max_length + 1);

        for (auto it = start; start != end; ++start)
        {
            if (islower(*it))
                continue;
            if (!isupper(*it))
                break;
            return {start, it};
        }

        return {};
    }

    string_view extract_exact(size_t length) const
    {
        assert(operator bool());

        auto start   = type_.data() + internal_prefix.size();
        auto abs_end = type_.data() + type_.size();
        auto end     = type_.data() + length + 1;

        return {start, end < abs_end && contains(length) ? end : start};
    }

    //

    string_view const *operator->() const
    {
        return &type_;
    }
};

static string_view check_int_prefix(prefix_string name)
{
    if (name && name.contains("uch"))
        return "valve::color";

    return {};
}

static string_view check_float_prefix(prefix_string name)
{
    if (name)
    {
#ifdef _DEBUG
        if (name.contains("rgfl"))
        {
            __debugbreak();
            return {};
        }
#endif

        auto prefix = name.extract_exact(3);
        if (prefix == "ang")
            return "valve::qangle";
        if (prefix == "vec")
            return "valve::vector3d";
    }
    return {};
}

static size_t array_length(valve::recv_prop *prop)
{
    assert(is_array(prop));
    size_t length = 0;

    if (auto parent = prop->parent_array_name)
    {
        do
        {
            ++length;
        }
        while ((++prop)->parent_array_name == parent);
    }
    else if (prop->name[0] == '[')
    {
        do
        {
            ++length;
        }
        while ((++prop)->name[0] == '[');
    }
    else
    {
        for (;;)
        {
            ++length;

            using pt = valve::recv_prop_type;
            switch ((++prop)->type)
            {
            case pt::DPT_Int:
            case pt::DPT_Float:
            case pt::DPT_Vector:
            case pt::DPT_VectorXY:
            case pt::DPT_String:
            case pt::DPT_Array:
            case pt::DPT_DataTable:
            case pt::DPT_Int64:
                continue;
            }

            break;
        }
    }

    return length;
}

static string_view netvar_type_by_prefix(valve::recv_prop *prop)
{
    using pt = valve::recv_prop_type;

    switch (prop->type)
    {
    case pt::DPT_Int:
        if (array_length(prop) == 3)
            return check_int_prefix(prop->name);
        break;
    case pt::DPT_Float:
        if (array_length(prop) == 3)
            return check_float_prefix(prop->name);
        break;
    }

    return {};
}

static string_view netvar_type_integer(prefix_string name)
{
    if (name)
    {
        auto prefix = name.extract(3);
        switch (prefix.size())
        {
        case 1: {
            if (prefix[0] == 'b')
                return "bool";
            if (prefix[0] == 'c')
                return "uint8_t";
            if (prefix[0] == 'h')
                return "valve::entity_handle";
            break;
        }
        case 2: {
            if (prefix == "un")
                return "uint32_t";
            if (prefix == "ch")
                return "uint8_t";
            if (prefix == "fl" && name->ends_with("Time")) //  SimulationTime int ???
                return "float";
            break;
        }
        case 3: {
            if (prefix == "clr")
                return "valve::color"; // not sure
            break;
        }
        default:
            std::unreachable();
        };
    }
    return "int32_t";
}

static string_view netvar_type_vec3(prefix_string name)
{
    if (name)
    {
        auto is_qangle = [name = string_view(name)] {
            auto real_name = name.substr(strlen("m_***"));
            if (real_name.size() >= strlen("angles"))
            {
                auto second_real_name_char = real_name.find("ngles");
                if (second_real_name_char != real_name.npos && second_real_name_char != 0)
                {
                    auto first_real_name_char = real_name[second_real_name_char - 1];
                    if (first_real_name_char == 'a' || first_real_name_char == 'A')
                        return true;
                }
            }
            assert(!isdigit(name[0]));
            return false;
        };

        if (name.contains("ang") || is_qangle())
            return "valve::qangle";
    }

    return "valve::vector3d";
}

static string_view netvar_type(valve::recv_prop *prop)
{
    using pt = valve::recv_prop_type;

    switch (prop->type)
    {
    case pt::DPT_Int:
        return netvar_type_integer(prop->name);
    case pt::DPT_Float:
        return "float";
    case pt::DPT_Vector:
        return netvar_type_vec3(prop->name);
    case pt::DPT_VectorXY:
        return "valve::vector2d"; // 3d vector. z unused
    case pt::DPT_String:
        return "char *"; // char[X]
    case pt::DPT_Array: {
        assert(0 && "Resolve array manually");
        return "void *";
    }
    case pt::DPT_DataTable: {
#if 0
        return prop->name;
#else
        assert(0 && "Data table type must be manually resolved!");
        return "void *";
#endif
    }
    case pt::DPT_Int64:
        return "int64_t";
    default: {
        assert(0 && "Unknown recv prop type");
        return "void *";
    }
    }
}

static string_view netvar_type(valve::data_map_description *field)
{
    using ft = valve::data_map_description_type;

    switch (field->type)
    {
    case ft::FIELD_VOID:
        return "void *";
    case ft::FIELD_FLOAT:
        return "float";
    case ft::FIELD_STRING:
        return "char *"; // string_t at real
    case ft::FIELD_VECTOR:
        return netvar_type_vec3(field->name);
    case ft::FIELD_QUATERNION:
        return "valve::quaternion";
    case ft::FIELD_INTEGER:
        return netvar_type_vec3(field->name);
    case ft::FIELD_BOOLEAN:
        return "bool";
    case ft::FIELD_SHORT:
        return "int16_t";
    case ft::FIELD_CHARACTER:
        return "int8_t";
    case ft::FIELD_COLOR32:
        return "valve::color";
    case ft::FIELD_EMBEDDED:
        assert(0 && "Embedded field detected");
        std::unreachable();
    case ft::FIELD_CUSTOM:
        assert(0 && "Custom field detected");
        std::unreachable();
    case ft::FIELD_CLASSPTR:
        return /*types_cache::simple(name,client_side::base_entity *)*/ "void *";
    case ft::FIELD_EHANDLE:
        return "valve::handle";
    case ft::FIELD_EDICT:
        assert(0 && "Edict field detected"); //  "edict_t*"
        std::unreachable();
    case ft::FIELD_POSITION_VECTOR:
        return "valve::vector3d";
    case ft::FIELD_TIME:
        return "float";
    case ft::FIELD_TICK:
        return "int32_t";
    case ft::FIELD_MODELNAME:
    case ft::FIELD_SOUNDNAME:
        return "char *"; // string_t at real
    case ft::FIELD_INPUT:
        assert(0 && "Inputvar field detected"); //  "CMultiInputVar"
        std::unreachable();
    case ft::FIELD_FUNCTION:
        assert(0 && "Function detected");
        std::unreachable();
    case ft::FIELD_VMATRIX:
    case ft::FIELD_VMATRIX_WORLDSPACE:
        return "valve::matrix4x4"; // VMatrix
    case ft::FIELD_MATRIX3X4_WORLDSPACE:
        return "valve::matrix3x4";
    case ft::FIELD_INTERVAL:
        assert(0 && "Interval field detected"); // "interval_t"
        std::unreachable();
    case ft::FIELD_MODELINDEX:
    case ft::FIELD_MATERIALINDEX:
        return "int32_t";
    case ft::FIELD_VECTOR2D:
        return "valve::vector2d";
    default:
        assert(0 && "Unknown datamap field type");
        std::unreachable();
    }
}

template <typename It>
static void netvar_type_array(valve::recv_prop *prop, It out)
{
    assert(is_array(prop));

    auto prev_prop      = prop - 1;
    auto prev_prop_type = netvar_type(prev_prop);

    fmt::format_to(out, "array<{}, {}>", prev_prop_type, prop->elements_count);
}

static void write_netvar_type(valve::recv_prop *prop, string &cache)
{
    assert(cache.empty());
    if (is_array(prop))
    {
        auto resolved = netvar_type_by_prefix(prop);
        if (resolved.empty())
            netvar_type_array(prop, std::back_inserter(cache));
        else
            cache.assign(resolved);
    }
    else
    {
        cache.assign(netvar_type(prop));
    }
}

static void write_netvar_type(valve::data_map_description *prop, string &cache)
{
    assert(cache.empty());
    cache.assign(netvar_type(prop));
}

string_view netvar_info::type() const
{
    return visit(
        overload(
            [](auto *prop, string const &cache) -> string_view {
                if (cache.empty())
                    write_netvar_type(prop, remove_const(cache));
                return {cache.begin(), cache.end()};
            },
            [](auto *prop, basic_netvar_type_cache *cache) {
                auto result = cache->get(prop);
                if (result.empty())
                {
                    string buff;
                    write_netvar_type(prop, buff);
                    result = cache->store(prop, std::move(buff));
                }
                return result;
            }),
        source_,
        type_);
}

size_t netvar_info::offset() const
{
    return visit(overload(&valve::recv_prop::offset, &valve::data_map_description::offset), source_);
}

netvar_table::netvar_table(netvar_table_source source, basic_netvar_type_cache *type_cache)
    : source_(source)
{
}

basic_netvar_table *netvar_table::inner(string_view name)
{
    // recursive storage not implemented
    (void)name;
    return nullptr;
}

using placeholders::_1;

basic_netvar_info *netvar_table::get(string_view name)
{
#ifdef _DEBUG
    if (storage_.empty())
        return nullptr;
#endif
    auto end = storage_.end();
    auto it  = std::find_if(storage_.begin(), end, _1->*&netvar_info::name == name);
    return it == end ? nullptr : &*it;
}

string_view netvar_table::raw_name() const
{
    return visit(overload(&valve::recv_table::name, &valve::data_map::name), source_);
}

string_view netvar_table::name() const
{
    return visit(
        overload(
            [](valve::recv_table *table) {
                auto table_name = table->name;
                return table_name[0] == 'D' ? table_name + strlen("DT_") : table_name;
            },
            _1->*&valve::data_map::name + strlen("C_")),
        source_);
}

bool netvar_table::empty() const
{
    return storage_.empty();
}

string_view netvar_type_cache::get(void *key) const
{
    string_view result;
    auto end = storage_.end();
    auto it  = std::find_if(storage_.begin(), end, _1->*&stored_value::key == key);
    if (it != end)
        result = it->view();
    return result;
}

string_view netvar_type_cache::store(void *key, string &&value)
{
    return storage_.emplace_back(key, std::move(value)).view();
}

basic_netvar_table *netvar_storage::get(string_view name)
{
    auto end = storage_.end();
    auto it  = std::find_if(storage_.begin(), end, _1->*&netvar_table::name == name);
    return it == end ? nullptr : &*it;
}

void netvar_storage::store(valve::client_class *root)
{
    for (; root != nullptr; root = root->next)
    {
        auto tmp = netvar_table(root->table, &class_types_);
        if (!tmp.empty())
            storage_.emplace_back(std::move(tmp));
    }
}

void netvar_storage::store(valve::data_map *root)
{
    for (; root != nullptr; root = root->base)
    {
        auto tmp = netvar_table(root, &datamap_types_);
        if (!tmp.empty())
            storage_.emplace_back(std::move(tmp));
    }
}
} // namespace fd