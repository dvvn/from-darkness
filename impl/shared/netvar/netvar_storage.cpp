#include "netvar_storage.h"
#include "preprocessor.h"
#include "variant.h"
#include "container/span.h"
#include "container/vector/dynamic.h"
#include "functional/ignore.h"
#include "functional/overload.h"
#include "functional/placeholders.h"
#include "iterator/unwrap.h"
#include "native/client_class.h"
#include "native/data_map.h"
#include "native/recv_table.h"
#include "string/char.h"
#include "string/dynamic.h"
#include "string/view.h"

#include <fmt/format.h>

#include <algorithm>
#include <cassert>

namespace fd
{
using netvar_source      = variant<native_recv_table::prop *, native_data_map::field *>;
using netvar_type_stored = variant<string, basic_netvar_type_cache *>;

using netvar_table_source = variant<native_recv_table *, native_data_map *>;

template <size_t S>
static bool strcmp_unsafe(char const *buff, char const (&cmp)[S])
{
    return buff[S] == '\0' && memcmp(buff, cmp, S - 1) == 0;
}

template <bool InFront>
static bool one_demension_array(string_view str)
{
    if constexpr (InFront)
    {
        if (!str.ends_with("[0]"))
            return false;
        return *std::next(str.rbegin(), 3) != ']';
    }
    else
    {
        if (!str.ends_with(']'))
            return false;
        auto open  = static_cast<string_view::difference_type>(str.rfind('['));
        auto start = next(str.begin(), open);
        if (*prev(start) == ']')
            return false;
        if (str.length() - 1 - open == 1)
            return isdigit(*start);
        return std::all_of(start, prev(str.end()), isdigit);
    }
}

static_assert(std::same_as<std::variant_alternative_t<1, netvar_type_stored>, basic_netvar_type_cache *>);

using v_prop_type  = native_recv_table::prop_type;
using v_field_type = native_data_map::field_type;

using placeholders::_1;

static char const *recv_prop_name(native_recv_table::prop *prop)
{
    using std::prev;

    if (isdigit(prop->name[0]))
        return prop->parent_array_name;
    if (prop->type == v_prop_type::array)
        return prev(prop)->name;
    return prop->name;
}

static string_view recv_prop_name_pretty(native_recv_table::prop *prop)
{
    if (isdigit(prop->name[0]))
        return prop->parent_array_name;
    if (prop->type == v_prop_type::array)
        return prop->name;

    string_view name(prop->name);
    if (one_demension_array<false>(name))
        name.remove_suffix(3);
    return name;
}

static string_view data_map_field_name_pretty(native_data_map::field *field)
{
    return field->name;
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
        return !extract_exact(length).empty();
    }

    bool contains(string_view prefix) const
    {
        assert(operator bool());

        auto start   = type_.data() + internal_prefix_.length();
        auto abs_end = type_.data() + type_.length();
        auto end     = start + prefix.length();
        if (abs_end <= end)
            return false;
        if (!isupper(*end))
            return false;
        if (!std::equal(start, end, prefix.data()))
            return false;

        return true;
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

    string_view extract() const
    {
        assert(operator bool());

        auto start = type_.data() + internal_prefix_.length();
        auto end   = type_.data() + type_.length();

        for (auto it = start; it != end; ++it)
        {
            if (islower(*it))
                continue;
            if (!isupper(*it))
                break;
            return {start, it};
        }

        return {};
    }

    string_view extract(size_t max_length) const
    {
        assert(operator bool());

        auto start   = type_.data() + internal_prefix_.length();
        auto abs_end = type_.data() + type_.length();
        auto end     = start + max_length + 1;
        if (abs_end < end)
            end = abs_end;

        for (auto it = start; it != end; ++it)
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
        auto end     = start + length;

        if (abs_end <= end)
            return {};
        if (!isupper(*end))
            return {};
        if (!std::all_of(start, end, islower))
            return {};

        return {start, end};
    }

    string_view const *operator->() const
    {
        return &type_;
    }
};

static string_view netvar_type_int32(netvar_prefix name)
{
    if (name)
    {
        auto prefix = name.extract(3);
        switch (prefix.size())
        {
        case 0:
            break;
        case 1:
        {
            if (prefix[0] == 'b')
                return "bool";
            if (prefix[0] == 'c')
                return "uint8_t";
            if (prefix[0] == 'h')
                return "valve::entity_handle";
            break;
        }
        case 2:
        {
            if (prefix == "un")
                return "uint32_t";
            if (prefix == "ch")
                return "uint8_t";
            if (prefix == "fl" && name->ends_with("Time")) //  SimulationTime int ???
                return "float";
            break;
        }
        case 3:
        {
            if (prefix == "clr")
                return "valve::color"; // not sure
            break;
        }
        default:
            unreachable();
        }
    }
    return "int32_t";
}

static string_view netvar_type_vec3(netvar_prefix name)
{
    if (name)
    {
        auto prefix = name.extract_exact(3);
        if (prefix == "ang")
            return "valve::qangle";
        if (prefix.empty() && name->contains("ngles"))
            return "valve::qangle";
#ifdef _DEBUG
        if (name.contains("rgfl"))
            __debugbreak();
#endif
    }

    return "valve::vector3d";
}

static string_view prop_type_vec2(native_recv_table::prop *prop)
{
    if (netvar_prefix name = prop->name)
    {
        auto prefix = name.extract_exact(3);
        if (prefix == "vec")
        {
            // fuck you valve
            auto next_prop = prop + 1;
            if (string_view(next_prop->name + name->length(), 3) == "[2]")
                return "valve::vector3d";
        }
    }

    return "valve::vector2d";
}

static string_view netvar_type(native_recv_table::prop *prop)
{
    switch (prop->type)
    {
    case v_prop_type::int32:
        return netvar_type_int32(prop->name);
    case v_prop_type::floating:
        return "float";
    case v_prop_type::vector3d:
        return netvar_type_vec3(prop->name);
    case v_prop_type::vector2d:
        return prop_type_vec2(prop); // 3d vector. z unused??
    case v_prop_type::string:
        return "char *"; // char[X]
    case v_prop_type::array:
    {
        assert(0 && "Resolve array manually");
        return "void *";
    }
    case v_prop_type::data_table:
    {
#if 0
        return prop->name;
#else
        assert(0 && "Data table type must be manually resolved!");
        return "void *";
#endif
    }
    case v_prop_type::int64:
        return "int64_t";
    default:
    {
        assert(0 && "Unknown recv prop type");
        return "void *";
    }
    }
}

static string_view netvar_type(native_data_map::field *field)
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
        return netvar_type_vec3(field->name);
    case v_field_type::quaternion:
        return "valve::quaternion";
    case v_field_type::integer:
        return netvar_type_int32(field->name);
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
        unreachable();
    case v_field_type::custom:
        assert(0 && "custom field detected");
        unreachable();
    case v_field_type::classptr:
        return /*types_cache::simple(name,client_side::base_entity *)*/ "void *";
    case v_field_type::ehandle:
        return "valve::handle";
    case v_field_type::edict:
        assert(0 && "edict field detected"); //  "edict_t*"
        unreachable();
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
        unreachable();
    case v_field_type::function:
        assert(0 && "function detected");
        unreachable();
    case v_field_type::vmatrix:
    case v_field_type::vmatrix_worldspace:
        return "valve::matrix4x4"; // vmatrix
    case v_field_type::matrix3x4_worldspace:
        return "valve::matrix3x4";
    case v_field_type::interval:
        assert(0 && "interval field detected"); // "interval_t"
        unreachable();
    case v_field_type::modelindex:
    case v_field_type::materialindex:
        return "int32_t";
    case v_field_type::vector2d:
        return "valve::vector2d";
    default:
        assert(0 && "unknown datamap field type");
        unreachable();
    }
}

template <typename It>
static void netvar_type_array(It out, string_view type, size_t length)
{
    fmt::format_to(out, "array<{}, {}>", type, length);
}

static string_view wrap_prop_in(native_recv_table::prop *prop)
{
    if (prop->type != v_prop_type::floating)
        return {};

    string_view prop_name(prop->name);

    auto splitted_array = [&]() -> size_t
    {
        if (!one_demension_array<true>(prop_name))
            return -1;
        auto num      = prop_name.length() - 2;
        auto num_c    = '0';
        size_t length = 1;
        for (auto p = prop; num_c != '3'; ++p)
        {
            if (p->name[num] != num_c)
                break;
            ++length;
            ++num_c;
        }
        return length;
    };

    auto named_vector = [&]() -> size_t
    {
        auto offset = prop_name.find('X');
        if (offset == prop_name.npos)
            return -1; // ignore _Y _Z by default
        if ((prop + 1)->name[offset] != 'Y')
            return 1;
        if ((prop + 2)->name[offset] != 'Z')
            return 2;
        if ((prop + 3)->name[offset] != 'W') // NOT SURE!!!
            return 3;
        return 4;
    };

    switch (splitted_array()) // why not qangle?
    {
    case 2:
        return "valve::vector2d";
    case 3:
        return "valve::vector3d";
    }

    switch (named_vector())
    {
    case 2:
        return "valve::vector2d";
    case 3:
        return "valve::vector3d";
    }

    return {};
}

static void write_netvar_type(native_recv_table::prop *prop, string &cache)
{
    assert(cache.empty());
    if (prop->type == v_prop_type::array)
    {
        // todo: say why prev prop?
        auto prev_prop = prop - 1;
        netvar_type_array(std::back_inserter(cache), netvar_type(prev_prop), prop->elements_count);
    }
    else if (prop->name[0] == '0')
    {
        auto type     = netvar_type(prop);
        // todo: get parent table!
        size_t length = 1;
        for (auto hint = prop->parent_array_name;;)
        {
            ++prop;
            if (prop->parent_array_name != hint)
                break;
            ++length;
        }
        if (length != 1)
            netvar_type_array(std::back_inserter(cache), type, length);
        else
            cache.assign(type).push_back('*');
    }
    else if (auto wrapped = wrap_prop_in(prop); !wrapped.empty())
    {
        cache.assign(wrapped);
    }
    else
    {
        cache.assign(netvar_type(prop));
    }
}

static void write_netvar_type(native_data_map::field *prop, string &cache)
{
    assert(cache.empty());
    cache.assign(netvar_type(prop));
}

static basic_netvar_type_cache::key_type key_for_cache(native_recv_table::prop *prop)
{
    return recv_prop_name(prop);
}

static basic_netvar_type_cache::key_type key_for_cache(native_data_map::field *dmap)
{
    return dmap->name;
}

class netvar_info final : public basic_netvar_info
{
    // friend class netvar_table;

    netvar_source source_;
    netvar_type_stored type_;
#ifdef FD_MERGE_NETVAR_TABLES
    size_t extra_offset_;
#endif

  public:
#ifdef FD_MERGE_NETVAR_TABLES
    netvar_info(netvar_source source, size_t extra_offset, basic_netvar_type_cache *type_cache)
        : source_(source)
        , type_(type_cache)
        , extra_offset_(extra_offset)
    {
        assert(type_cache != nullptr);
    }

    netvar_info(netvar_source source, size_t extra_offset)
        : source_(source)
        , extra_offset_(extra_offset)
    {
    }
#else
    netvar_info(netvar_source source, basic_netvar_type_cache *type_cache)
        : source_(source)
        , type_(type_cache)
    {
        assert(type_cache != nullptr);
    }

    netvar_info(netvar_source source)
        : source_(source)
    {
    }
#endif

    char const *raw_name() const
    {
        return visit(overload(&native_recv_table::prop::name, &native_data_map::field::name), source_);
    }

    size_t raw_offset() const
    {
        return visit(overload(&native_recv_table::prop::offset, &native_data_map::field::offset), source_);
    }

    char const *name() const
    {
        return visit(overload(recv_prop_name, &native_data_map::field::name), source_);
    }

    /**
     * \brief dont compare internal names with it!!!
     */
    string_view pretty_name() const
    {
        return visit(overload(recv_prop_name_pretty, data_map_field_name_pretty), source_);
    }

    size_t offset() const override
    {
        return
#ifdef FD_MERGE_NETVAR_TABLES
            extra_offset_ +
#endif
            raw_offset();
    }

    string_view type() const
    {
        return visit(
            overload(
                [](auto *prop, string const &cache) -> string_view
                {
                    if (cache.empty())
                        write_netvar_type(prop, const_cast<string &>(cache));
                    return {cache.begin(), cache.end()};
                },
                [](auto *prop, basic_netvar_type_cache *cache)
                {
                    auto key = key_for_cache(prop);
                    if (auto result = cache->get(key); !result.empty())
                        return result;
                    string buff;
                    write_netvar_type(prop, buff);
                    return cache->store(key, std::move(buff));
                }),
            source_,
            type_);
    }
};

class netvar_table final : public basic_netvar_table
{
    friend class netvar_storage;

    using view_type = span<netvar_info const>;

    netvar_table_source source_;
    vector<netvar_info> storage_;
#ifndef FD_MERGE_NETVAR_TABLES
    vector<netvar_table> inner_;
#endif

    void parse_recv_table(
        native_recv_table *recv, size_t offset, basic_netvar_type_cache *type_cache, bool filter_duplicates)
    {
        if (recv->props_count == 0)
            return;

        auto prop = recv->props;

        auto store = [&]<bool Cache>(std::bool_constant<Cache>)
        {
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
        };

        if (strcmp_unsafe(prop->name, "baseclass"))
        {
            if (recv->props_count == 1)
                return;
            ++prop;
        }

        if (prop->name[0] == '0')
        {
            // calculate array length and skip it
#ifdef FD_MERGE_NETVAR_TABLES
            if (!filter_duplicates ||
                std::all_of(storage_.begin(), storage_.end(), _1->*&netvar_info::name != prop->parent_array_name))
            {
                if (type_cache)
                    store(std::true_type());
                else
                    store(std::false_type());
            }
#endif
            return;
        }
        else if ( // NOLINT(readability-else-after-return)
            prop->array_length_proxy || strcmp_unsafe(prop->name, "lengthproxy"))
        {
            // WIP
            return;
        }

        auto do_parse =
            [&]<bool Cache, bool Duplicates>(std::bool_constant<Cache> cache, std::bool_constant<Duplicates>)
        {
            auto props_end = recv->props + recv->props_count;
            for (; prop != props_end; ++prop)
            {
                if (prop->type != v_prop_type::data_table)
                {
                    if constexpr (Duplicates)
                    {
                        if (std::any_of(storage_.begin(), storage_.end(), _1->*&netvar_info::raw_name == prop->name))
                            continue;
                    }
                    store(cache);
                }
                else if (prop->data_table)
                {
#ifdef FD_MERGE_NETVAR_TABLES
                    parse_recv_table(prop->data_table, offset + prop->offset, type_cache, !storage_.empty());
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

    void parse_data_map(
        native_data_map *dmap, size_t offset, basic_netvar_type_cache *type_cache, bool filter_duplicates)
    {
        auto do_parse = [=]<bool Cache, bool Duplicates>(std::bool_constant<Cache>, std::bool_constant<Duplicates>)
        {
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
                    // todo: sort by offset
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

    /**
     * \brief for internal use only
     * \param name pointer to other netvar name stored in game's memory
     */
    basic_netvar_info *get_raw(char const *name)
    {
        auto end = storage_.end();
        auto it  = std::find_if(storage_.begin(), end, _1->*&netvar_info::raw_name == name);
        return it == end ? nullptr : iterator_to_raw_pointer(it);
    }

  public:
    netvar_table(netvar_table_source source, basic_netvar_type_cache *type_cache)
        : source_(source)
    {
        visit(
            bind(
                overload(&netvar_table::parse_recv_table, &netvar_table::parse_data_map),
                FD_GROUP_ARGS(this, _1, 0u, type_cache, 0u)),
            source_);
    }

    basic_netvar_table *inner(string_view name) override
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

    basic_netvar_info *get(string_view name) override
    {
#ifdef _DEBUG
        if (storage_.empty())
            return nullptr;
#endif
        auto end = storage_.end();
        auto it  = std::find_if(storage_.begin(), end, _1->*&netvar_info::pretty_name == name);
        return it == end ? nullptr : iterator_to_raw_pointer(it);
    }

    view_type view() const
    {
        return storage_;
    }

    char const *raw_name() const
    {
        return visit(overload(&native_recv_table::name, &native_data_map::name), source_);
    }

    char const *name() const
    {
        return visit(
            overload(
                [](native_recv_table *table)
                {
                    auto table_name = table->name;
                    return table_name[0] == 'D' ? table_name + strlen("DT_") : table_name;
                },
                _1->*&native_data_map::name + strlen("C_")),
            source_);
    }

    bool empty() const
    {
        return storage_.empty();
    }
};

static void try_write_netvar_table(auto &storage, auto... args)
{
    netvar_table table(args...);
    if (!table.empty())
        storage.emplace_back(std::move(table));
}

class netvar_type_cache final : public basic_netvar_type_cache
{
    struct stored_value
    {
        key_type key;
        string type;

        string_view view() const
        {
            return {type.begin(), type.end()};
        }
    };

    vector<stored_value> storage_;

  public:
    string_view get(key_type key) const override
    {
        string_view result;
        auto end = storage_.end();
        auto it  = std::find_if(storage_.begin(), end, _1->*&stored_value::key == key);
        if (it != end)
            result = it->view();
        return result;
    }

    string_view store(key_type key, string &&value) override
    {
        return storage_.emplace_back(key, move(value)).view();
    }
};

static netvar_table *find_netvar_table(string_view name, auto &storage)
{
    auto end = storage.end();
    auto it  = std::find_if(storage.begin(), end, _1->*&netvar_table::name == name);
    return it == end ? nullptr : iterator_to_raw_pointer(it);
}

template <bool Raw, typename T, typename... Next>
static netvar_table *find_netvar_table(string_view name, T  &storage, Next &...next)
{
    // static_assert(!Raw || sizeof...(Next) == 0);
    {
        auto end = storage.end();
        typename T::iterator it;
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

class netvar_storage final : public basic_netvar_storage
{
    struct data_storage : vector<netvar_table>
    {
        netvar_type_cache types;
    };

    data_storage recv_tables_;
    data_storage data_maps_;

  public:
    basic_netvar_table *get(string_view name) override
    {
        if (name.starts_with("DT_"))
            return find_netvar_table<true>(name, recv_tables_ /*, data_maps_*/);

        if (name.starts_with("C_"))
            return find_netvar_table<true>(name, data_maps_ /*, data_tables_*/);

        return find_netvar_table<false>(name, data_maps_, recv_tables_);
    }

    void store(native_client_class *root) override
    {
        for (; root != nullptr; root = root->next)
            try_write_netvar_table(recv_tables_, root->table, &recv_tables_.types);
    }

    void store(native_data_map *root) override
    {
        if (data_maps_.empty())
        {
            for (; root != nullptr; root = root->base)
                try_write_netvar_table(data_maps_, root, &data_maps_.types);
        }
        else
        {
            for (; root != nullptr; root = root->base)
            {
                auto end = data_maps_.end();
#if 0
            auto existing = std::find_if(data_maps_.begin(), end, _1->*&netvar_table::raw_name == root->name);
#else // skip inner visit
                auto existing = std::find_if(
                    data_maps_.begin(),
                    end,
                    [root_name = root->name](netvar_table &table)
                    { return std::get<native_data_map *>(table.source_)->name == root_name; });
#endif
                if (existing == end)
                    try_write_netvar_table(data_maps_, root, &data_maps_.types);
                else
                    existing->parse_data_map(root, 0, &data_maps_.types, true);
            }
        }
    }
};

FD_OBJECT_IMPL(netvar_storage);
} // namespace fd