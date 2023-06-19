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

static_assert(std::same_as<std::variant_alternative_t<1, netvar_type_stored>, basic_netvar_type_cache *>);

using v_prop_type  = valve::recv_prop_type;
using v_field_type = valve::data_map_field_type;

#ifdef FD_MERGE_NETVAR_TABLES
netvar_info::netvar_info(netvar_source source, size_t extra_offset, basic_netvar_type_cache *type_cache)
    : source_(source)
    , type_(type_cache)
    , extra_offset_(extra_offset)
{
    assert(type_cache != nullptr);
}

netvar_info::netvar_info(netvar_source source, size_t extra_offset)
    : source_(source)
    , extra_offset_(extra_offset)
{
}
#else
netvar_info::netvar_info(netvar_source source, basic_netvar_type_cache *type_cache)
    : source_(source)
    , type_(type_cache)
{
    assert(type_cache != nullptr);
}

netvar_info::netvar_info(netvar_source source)
    : source_(source)
{
}
#endif

char const *netvar_info::raw_name() const
{
    return visit(overload(&valve::recv_prop::name, &valve::data_map_field::name), source_);
}

size_t netvar_info::raw_offset() const
{
    return visit(overload(&valve::recv_prop::offset, &valve::data_map_field::offset), source_);
}

char const *netvar_info::name() const
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
            &valve::data_map_field::name),
        source_);
}

size_t netvar_info::offset() const
{
    return
#ifdef FD_MERGE_NETVAR_TABLES
        extra_offset_ +
#endif
        raw_offset();
}

static bool is_array(valve::recv_prop *prop)
{
    return prop->type == v_prop_type::array;
}

class netvar_prefix
{
    static constexpr string_view internal_prefix_ = "m_";
    string_view type_;

  public:
    netvar_prefix(char const *type)
        : type_(type)
    {
    }

    netvar_prefix(string_view type)
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

        auto tmp = type_.substr(internal_prefix_.length());

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

        auto tmp = type_.substr(internal_prefix_.length());
        if (tmp.length() <= prefix.length() + 1)
            return false;
        if (!std::isupper(tmp[prefix.length()]))
            return false;

        return tmp.starts_with(prefix);
    }

    explicit operator bool() const
    {
        // m_xX
        if (type_.size() < internal_prefix_.size() + 2)
            return false;
        if (!type_.starts_with(internal_prefix_))
            return false;
        return true;
    }

    string_view extract(size_t max_length = std::numeric_limits<size_t>::max() - 1) const
    {
        assert(operator bool());

        auto start = type_.data() + internal_prefix_.size();
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

        auto start   = type_.data() + internal_prefix_.size();
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

static string_view check_int_prefix(netvar_prefix name)
{
    if (name && name.contains("uch"))
        return "valve::color";

    return {};
}

static string_view check_float_prefix(netvar_prefix name)
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
            auto type = (++prop)->type;
            if (type < v_prop_type::int32 || type > v_prop_type::int64)
                break;
        }
    }

    return length;
}

static string_view netvar_type_by_prefix(valve::recv_prop *prop)
{
    switch (prop->type)
    {
    case v_prop_type::int32:
        if (array_length(prop) == 3)
            return check_int_prefix(prop->name);
        break;
    case v_prop_type::floating:
        if (array_length(prop) == 3)
            return check_float_prefix(prop->name);
        break;
    }

    return {};
}

static string_view prop_type_int32(netvar_prefix name)
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

static string_view prop_type_vec3(netvar_prefix name)
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
    switch (prop->type)
    {
    case v_prop_type::int32:
        return prop_type_int32(prop->name);
    case v_prop_type::floating:
        return "float";
    case v_prop_type::vector3d:
        return prop_type_vec3(prop->name);
    case v_prop_type::vector2d:
        return "valve::vector2d"; // 3d vector. z unused
    case v_prop_type::string:
        return "char *"; // char[X]
    case v_prop_type::array: {
        assert(0 && "Resolve array manually");
        return "void *";
    }
    case v_prop_type::data_table: {
#if 0
        return prop->name;
#else
        assert(0 && "Data table type must be manually resolved!");
        return "void *";
#endif
    }
    case v_prop_type::int64:
        return "int64_t";
    default: {
        assert(0 && "Unknown recv prop type");
        return "void *";
    }
    }
}

static string_view netvar_type(valve::data_map_field *field)
{
    switch (field->type)
    {
    case v_field_type::void_:
        return "void *";
    case v_field_type::floating:
        return "float";
    case v_field_type::string:
        return "char *"; // string_t at real
    case v_field_type::vector:
        return prop_type_vec3(field->name);
    case v_field_type::quaternion:
        return "valve::quaternion";
    case v_field_type::integer:
        return prop_type_vec3(field->name);
    case v_field_type::boolean:
        return "bool";
    case v_field_type::short_:
        return "int16_t";
    case v_field_type::character:
        return "int8_t";
    case v_field_type::color32:
        return "valve::color";
    case v_field_type::embedded:
        assert(0 && "embedded field detected");
        std::unreachable();
    case v_field_type::custom:
        assert(0 && "custom field detected");
        std::unreachable();
    case v_field_type::classptr:
        return /*types_cache::simple(name,client_side::base_entity *)*/ "void *";
    case v_field_type::ehandle:
        return "valve::handle";
    case v_field_type::edict:
        assert(0 && "edict field detected"); //  "edict_t*"
        std::unreachable();
    case v_field_type::position_vector:
        return "valve::vector3d";
    case v_field_type::time:
        return "float";
    case v_field_type::tick:
        return "int32_t";
    case v_field_type::modelname:
    case v_field_type::soundname:
        return "char *"; // string_t at real
    case v_field_type::input:
        assert(0 && "inputvar field detected"); //  "cmultiinputvar"
        std::unreachable();
    case v_field_type::function:
        assert(0 && "function detected");
        std::unreachable();
    case v_field_type::vmatrix:
    case v_field_type::vmatrix_worldspace:
        return "valve::matrix4x4"; // vmatrix
    case v_field_type::matrix3x4_worldspace:
        return "valve::matrix3x4";
    case v_field_type::interval:
        assert(0 && "interval field detected"); // "interval_t"
        std::unreachable();
    case v_field_type::modelindex:
    case v_field_type::materialindex:
        return "int32_t";
    case v_field_type::vector2d:
        return "valve::vector2d";
    default:
        assert(0 && "unknown datamap field type");
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

static void write_netvar_type(valve::data_map_field *prop, string &cache)
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

static void try_write_netvar_table(auto &storage, auto... args)
{
    netvar_table table(args...);
    if (table.empty())
        storage.emplace_back(std::move(table));
}

template <size_t S>
static bool strcmp_unsafe(char const *buff, char const (&cmp)[S])
{
    return buff[S] == '\0' && memcmp(buff, cmp, S - 1) == 0;
}

using placeholders::_1;

void netvar_table::parse_recv_table(
    valve::recv_table *recv,
    size_t offset,
    basic_netvar_type_cache *type_cache,
    bool filter_duplicates)
{
    if (recv->props_count == 0)
        return;

    auto prop = recv->props;

    if (strcmp_unsafe(prop->name, "baseclass"))
    {
        if (recv->props_count == 1)
            return;
        ++prop;
    }

    if (prop->name[0] == '0')
    { // NOLINT(bugprone-branch-clone)
        // WIP
        // calculate array length and skip it
        return;
    }
    else if ( // NOLINT(readability-else-after-return)
        prop->array_length_proxy || strcmp_unsafe(prop->name, "lengthproxy"))
    {
        // WIP
        return;
    }

    auto do_parse = [&]<bool Cache, bool Duplicates>(std::bool_constant<Cache>, std::bool_constant<Duplicates>) {
        auto props_end = recv->props + recv->props_count;
        for (; prop != props_end; ++prop)
        {
            if (prop->type != v_prop_type::data_table)
            {
                if constexpr (Duplicates)
                {
                    // DEBUG THIS!!!
                    if (std::any_of(storage_.begin(), storage_.end(), _1->*&netvar_info::raw_name == prop->name))
                        continue;
                }
#ifdef FD_MERGE_NETVAR_TABLES
                if constexpr (Cache)
                    storage_.emplace_back(prop, offset, type_cache);
                else
                    storage_.emplace_back(prop, offset);
#else
                if constexpr (Cache)
                    storage_.emplace_back(prop, type_cache);
                else
                    storage_.emplace_back(prop);
#endif
            }
            else if (prop->data_table)
            {
#ifdef FD_MERGE_NETVAR_TABLES
                parse_recv_table(prop->data_table, offset + prop->offset, type_cache, true);
#else
                try_write_netvar_table(innter_, prop->data_table, type_cache);
#endif
            }
        }
    };

    // filter also possible without 'filter_duplicates'
    // when offset != 0

    if (type_cache && filter_duplicates)
        do_parse(std::true_type(), std::true_type());
    else if (type_cache)
        do_parse(std::true_type(), std::false_type());
    else if (filter_duplicates)
        do_parse(std::false_type(), std::true_type());
    else
        do_parse(std::false_type(), std::false_type());
}

void netvar_table::parse_data_map(
    valve::data_map *dmap,
    size_t offset,
    basic_netvar_type_cache *type_cache,
    bool filter_duplicates)
{
    auto do_parse = [=]<bool Cache, bool Duplicates>(std::bool_constant<Cache>, std::bool_constant<Duplicates>) {
        auto fields_end = dmap->fields + dmap->fields_count;
        for (auto field = dmap->fields; field != fields_end; ++field)
        {
            if (field->type == v_field_type::embedded)
            {
                assert(!field->description); // Embedded datamap not implemented
            }
            else
            {
                if constexpr (Duplicates)
                {
                    if (this->get_raw(field->name))
                        continue;
                }

                if constexpr (Cache)
#ifdef FD_MERGE_NETVAR_TABLES
                    storage_.emplace_back(field, offset, type_cache);
                else
                    storage_.emplace_back(field, offset);
#else
                    storage_.emplace_back(field, type_cache);
                else
                    storage_.emplace_back(field);
#endif
            }

#ifdef _DEBUG
            if constexpr (Duplicates)
            {
                // sort by offset
            }
#endif
        }
    };

    if (type_cache && filter_duplicates)
        do_parse(std::true_type(), std::true_type());
    else if (type_cache)
        do_parse(std::true_type(), std::false_type());
    else if (filter_duplicates)
        do_parse(std::false_type(), std::true_type());
    else
        do_parse(std::false_type(), std::false_type());
}

netvar_table::netvar_table(netvar_table_source source, basic_netvar_type_cache *type_cache)
    : source_(source)
{
    visit(
        bind(overload(&netvar_table::parse_recv_table, &netvar_table::parse_data_map), this, _1, 0u, type_cache, 0u),
        source_);
}

basic_netvar_table *netvar_table::inner(string_view name)
{
#ifndef FD_MERGE_NETVAR_TABLES
    auto end = inner_.end();
    auto it  = std::find_if(inner_.begin(), end, _1->*&netvar_table::name == name);

    if (it != end)
        return iterator_to_raw_pointer(it);
#else
    ignore_unused(this, name);
#endif
    return nullptr;
}

basic_netvar_info *netvar_table::get(string_view name)
{
#ifdef _DEBUG
    if (storage_.empty())
        return nullptr;
#endif
    auto end = storage_.end();
    auto it  = std::find_if(storage_.begin(), end, _1->*&netvar_info::name == name);
    return it == end ? nullptr : iterator_to_raw_pointer(it);
}

basic_netvar_info *netvar_table::get_raw(char const *name)
{
    auto end = storage_.end();
    auto it  = std::find_if(storage_.begin(), end, _1->*&netvar_info::raw_name == name);
    return it == end ? nullptr : iterator_to_raw_pointer(it);
}

char const *netvar_table::raw_name() const
{
    return visit(overload(&valve::recv_table::name, &valve::data_map::name), source_);
}

char const *netvar_table::name() const
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

static netvar_table *find_netvar_table(string_view name, auto &storage)
{
    auto end = storage.end();
    auto it  = std::find_if(storage.begin(), end, _1->*&netvar_table::name == name);
    return it == end ? nullptr : iterator_to_raw_pointer(it);
}

template <bool Raw, typename T, typename... Next>
static netvar_table *find_netvar_table(string_view name, T &storage, Next &...next)
{
    // static_assert(!Raw || sizeof...(Next) == 0);
    {
        auto end = storage.end();
        decltype(end) it;
        if constexpr (Raw)
            it = std::find_if(storage.begin(), end, _1->*&netvar_table::raw_name == name);
        else
            it = std::find_if(storage.begin(), end, _1->*&netvar_table::name == name);

        if (it != end)
            return iterator_to_raw_pointer(it);
    }
    if constexpr (sizeof...(Next) == 0)
        return nullptr;
    else
        return find_netvar_table<Raw>(name, next...);
}

basic_netvar_table *netvar_storage::get(string_view name)
{
    if (name.starts_with("DT_"))
        return find_netvar_table<true>(name, data_tables_.data /*, data_maps_.data*/);

    if (name.starts_with("C_"))
        return find_netvar_table<true>(name, data_maps_.data /*, data_tables_.data*/);

    return find_netvar_table<false>(name, data_maps_.data, data_tables_.data);
}

void netvar_storage::store(valve::client_class *root)
{
    for (; root != nullptr; root = root->next)
        try_write_netvar_table(data_tables_.data, root->table, &data_tables_.types);
}

void netvar_storage::store(valve::data_map *root)
{
    if (data_maps_.data.empty())
    {
        for (; root != nullptr; root = root->base)
            try_write_netvar_table(data_maps_.data, root, &data_maps_.types);
    }
    else
    {
        for (; root != nullptr; root = root->base)
        {
            auto end = data_maps_.data.end();
#if 0
            auto existing = std::find_if(data_maps_.data.begin(), end, _1->*&netvar_table::raw_name == root->name);
#else // skip inner visit
            auto existing = std::find_if(data_maps_.data.begin(), end, [root_name = root->name](netvar_table &table) {
                return std::get<valve::data_map *>(table.source_)->name == root_name;
            });
#endif
            if (existing == end)
                try_write_netvar_table(data_maps_.data, root, &data_maps_.types);
            else
                existing->parse_data_map(root, 0, &data_maps_.types, true);
        }
    }
}
} // namespace fd