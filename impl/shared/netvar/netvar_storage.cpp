#include "netvar_storage.h"
#include "container/array.h"
#include "container/vector/dynamic.h"
#include "functional/cast.h"
#include "functional/ignore.h"
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

#define FD_MERGE_NETVAR_TABLES

namespace fd
{
class basic_netvar_types_cache
{
  protected:
    ~basic_netvar_types_cache() = default;

  public:
    using key_type    = void const *;
    using buffer_type = string;

    virtual string_view get(key_type key) const                  = 0;
    virtual string_view store(key_type key, buffer_type &&value) = 0;
};

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
        auto const open  = static_cast<string_view::difference_type>(str.rfind('['));
        auto const start = next(str.begin(), open);
        if (*prev(start) == ']')
            return false;
        if (str.length() - 1 - open == 1)
            return isdigit(*start);
        return std::all_of(start, prev(str.end()), isdigit);
    }
}

using v_prop_type  = native_recv_table::prop_type;
using v_field_type = native_data_map::field_type;

inline constexpr string_view class_prefix   = "C_";
inline constexpr string_view datamap_prefix = "DT_";

using placeholders::_1;
using placeholders::_2;

static char const *recv_prop_name(native_recv_table::prop const *prop)
{
    using std::prev;

    if (isdigit(prop->name[0]))
        return prop->parent_array_name;
    if (prop->type == v_prop_type::array)
        return prev(prop)->name;
    return prop->name;
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

    netvar_prefix(string_view const type)
        : type_(type)
    {
    }

    operator string_view() const
    {
        return type_;
    }

    bool contains(size_t const length) const
    {
        return !extract_exact(length).empty();
    }

    bool contains(string_view const prefix) const
    {
        assert(operator bool());

        auto const start   = type_.data() + internal_prefix_.length();
        auto const abs_end = type_.data() + type_.length();
        auto const end     = start + prefix.length();
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

        auto start     = type_.data() + internal_prefix_.length();
        auto const end = type_.data() + type_.length();

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

    string_view extract(size_t const max_length) const
    {
        assert(operator bool());

        auto start         = type_.data() + internal_prefix_.length();
        auto const abs_end = type_.data() + type_.length();
        auto end           = start + max_length + 1;
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

    string_view extract_exact(size_t const length) const
    {
        assert(operator bool());

        auto start         = type_.data() + internal_prefix_.size();
        auto const abs_end = type_.data() + type_.size();
        auto end           = start + length;

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

static string_view netvar_type_int32(netvar_prefix const name)
{
    if (name)
    {
        auto const prefix = name.extract(3);
        switch (prefix.size())
        {
        case 0:
            break;
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
            unreachable();
        }
    }
    return "int32_t";
}

static string_view netvar_type_vec3(netvar_prefix const name)
{
    if (name)
    {
        auto const prefix = name.extract_exact(3);
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

static string_view prop_type_vec2(native_recv_table::prop const *prop)
{
    if (netvar_prefix const name = prop->name)
    {
        auto const prefix = name.extract_exact(3);
        if (prefix == "vec")
        {
            // fuck you valve
            auto const next_prop = prop + 1;
            if (string_view(next_prop->name + name->length(), 3) == "[2]")
                return "valve::vector3d";
        }
    }

    return "valve::vector2d";
}

static string_view netvar_type(native_recv_table::prop const *prop)
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

static string_view netvar_type(native_data_map::field const *field)
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

static string_view wrap_prop_in(native_recv_table::prop const *prop)
{
    if (prop->type != v_prop_type::floating)
        return {};

    string_view prop_name(prop->name);

    auto splitted_array = [&]() -> size_t {
        if (!one_demension_array<true>(prop_name))
            return -1;
        auto const num = prop_name.length() - 2;
        auto num_c     = '0';
        size_t length  = 1;
        for (auto p = prop; num_c != '3'; ++p)
        {
            if (p->name[num] != num_c)
                break;
            ++length;
            ++num_c;
        }
        return length;
    };

    auto named_vector = [&]() -> size_t {
        auto const offset = prop_name.find('X');
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

static basic_netvar_types_cache::key_type key_for_cache(native_recv_table::prop const *prop)
{
    return recv_prop_name(prop);
}

static basic_netvar_types_cache::key_type key_for_cache(native_data_map::field const *dmap)
{
    return dmap->name;
}

static char const *netvar_info_name(native_recv_table::prop const *prop)
{
    return recv_prop_name(prop);
}

static char const *netvar_info_name(native_data_map::field const *field)
{
    return field->name;
}

static string_view netvar_info_pretty_name(native_recv_table::prop const *prop)
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

static string_view netvar_info_pretty_name(native_data_map::field const *field)
{
    return field->name;
}

static string netvar_info_type(native_recv_table::prop *prop)
{
    using std::back_inserter;
    using std::prev;

    string buffer;
    if (prop->type == v_prop_type::array)
    {
        // todo: say why prev prop?
        auto const prev_prop = prev(prop);
        netvar_type_array(back_inserter(buffer), netvar_type(prev_prop), prop->elements_count);
    }
    else if (prop->name[0] == '0')
    {
        // todo: get parent table!
        size_t length = 1;
        while (prop[length].parent_array_name == prop->parent_array_name)
            ++length;

        auto const type = netvar_type(prop);
        if (length != 1)
            netvar_type_array(back_inserter(buffer), type, length);
        else
        {
            constexpr string_view pointer = " *";
            buffer.reserve(type.length() + pointer.length());
            buffer.append(type).append(pointer);
        }
    }
    else if (auto const wrapped = wrap_prop_in(prop); !wrapped.empty())
    {
        buffer.assign(wrapped);
    }
    else
    {
        buffer.assign(netvar_type(prop));
    }

    return buffer;
}

static string_view netvar_info_type(native_data_map::field const *field)
{
    return netvar_type(field);
}

template <class T>
struct netvar_info final : basic_netvar_info
{
    using pointer       = T const *;
    using const_pointer = T const *;

  private:
    pointer source_;
#ifdef FD_MERGE_NETVAR_TABLES
    size_t extra_offset_;
#endif

  public:
#ifdef _DEBUG
    netvar_info() = default;
#else
    netvar_info()
    {
        ignore_unused(this);
    }
#endif

#ifdef FD_MERGE_NETVAR_TABLES
    netvar_info(pointer const source, size_t const extra_offset)
        : source_(source)
        , extra_offset_(extra_offset)
    {
    }
#else
    netvar_info(pointer source)
        : source_(source)
    {
    }
#endif

    char const *raw_name() const
    {
        return source_->name;
    }

    size_t raw_offset() const
    {
        return source_->offset;
    }

    char const *name() const
    {
        return netvar_info_name(source_);
    }

    /**
     * \brief dont compare internal names with it!!!
     */
    string_view pretty_name() const
    {
        return netvar_info_pretty_name(source_);
    }

    size_t offset() const override
    {
        return
#ifdef FD_MERGE_NETVAR_TABLES
            extra_offset_ +
#endif
            raw_offset();
    }

    auto type() const
    {
        return netvar_info_type(source_);
    }

    string_view type(basic_netvar_types_cache *cache) const
    {
        auto const key = key_for_cache(source_);
        auto const str = cache->get(key);
        return str.empty() ? cache->store(key, type()) : str;
    }
};

static char const *netvar_table_name(native_recv_table const *table)
{
    auto const name = table->name;
    if (name[0] != 'D')
        return name;
    assert(name[1] == 'T');
    assert(name[2] == '_');
    return name + datamap_prefix.length();
}

static char const *netvar_table_name(native_data_map const *map)
{
    auto const name = map->name;
    if (name[0] != 'C')
        return name;
    assert(name[1] == '_');
    return name + class_prefix.length();
}

template <uint8_t LastIsAlreadyBack = 2, class T>
static void move_last(vector<netvar_info<T>> &storage, netvar_info<T> &last)
{
    assert(!storage.empty());
    if (storage.size() == 1)
        return;

    auto do_move = [&]<bool LastIsBack>(std::bool_constant<LastIsBack>) {
        auto const end   = storage.rend();
        auto const begin = storage.rbegin();
        auto const pos   = find_if(
            next(begin, LastIsBack), end, //
            _1->*&netvar_info<T>::offset < last.offset());

        if constexpr (LastIsBack)
            if (distance(begin, pos) == 1)
                return;

        auto fwd_pos = prev(pos.base());

        if constexpr (LastIsBack)
        {
            auto skip_last = prev(storage.end());
            auto backup    = std::move(last);
            std::move_backward(fwd_pos, skip_last, next(fwd_pos));
            *fwd_pos = std::move(backup);
        }
        else
        {
            if (pos != end && storage.capacity() >= storage.size() + 1)
            {
                storage.emplace_back();
                auto skip_added = prev(storage.end());
                std::move_backward(fwd_pos, skip_added, next(fwd_pos));
            }
            else
            {
                vector<netvar_info<T>> new_storage;
                new_storage.resize(storage.size() + 1);
                auto inner = std::move(storage.begin(), next(fwd_pos), new_storage.begin());
                *inner     = last;
                std::move(next(fwd_pos), storage.end(), next(inner));
                storage.swap(new_storage);
            }
        }
    };

    if (LastIsAlreadyBack == 1 || (LastIsAlreadyBack != 0 && &last == &storage.back()))
        do_move(std::true_type());
    else
        do_move(std::false_type());
}

static void netvar_table_parse(
    vector<netvar_info<native_recv_table::prop>> &storage, //
    native_recv_table const *table, size_t const offset, bool const filter_duplicates)
{
    using netvar_info = netvar_info<native_recv_table::prop>;

    if (table->props_count == 0)
        return;

    auto prop = table->props;
    auto const store =
#ifdef FD_MERGE_NETVAR_TABLES
        [&storage, &prop, offset]() -> netvar_info & {
        return storage.emplace_back(prop, offset);
    };
#else
        [&]() -> netvar_info & {
        return storage.emplace_back(prop);
    };
#endif

    if (strcmp_unsafe(prop->name, "baseclass"))
    {
        if (table->props_count == 1)
            return;
        ++prop;
    }

    if (prop->name[0] == '0')
    {
        // calculate array length and skip it
#ifdef FD_MERGE_NETVAR_TABLES
        if (!filter_duplicates || //
            std::all_of(
                storage.cbegin(), storage.cend(), //
                _1->*&netvar_info::name != prop->parent_array_name))
        {
            store();
        }
#endif
        return;
    }
    if (prop->array_length_proxy || strcmp_unsafe(prop->name, "lengthproxy"))
    {
        // WIP
        return;
    }

    auto const do_parse = [&storage, &prop, &store, //
                           props_end = table->props + table->props_count,
                           offset]<bool Duplicates>(std::bool_constant<Duplicates>) {
        for (; prop != props_end; ++prop)
        {
            if (prop->type != v_prop_type::data_table)
            {
                if constexpr (Duplicates)
                {
                    if (std::any_of(
                            storage.cbegin(), storage.cend(), //
                            _1->*&netvar_info::raw_name == prop->name))
                        continue;
                }
                [[maybe_unused]] //
                auto &new_item = store();
#ifdef _DEBUG
                if constexpr (Duplicates)
                    move_last<true>(storage, new_item);
#endif
            }
            else if (prop->data_table)
            {
#ifdef FD_MERGE_NETVAR_TABLES
                netvar_table_parse(storage, prop->data_table, offset + prop->offset, !storage.empty());
#else
                NOT IMPLEMENTED
                // try_write_netvar_table(innter_, prop->data_table, type_cache);
#endif
            }
        }
    };

    // filter also possible without 'filter_duplicates'
    // when offset != 0

    if (filter_duplicates)
        do_parse(std::true_type());
    else
        do_parse(std::false_type());
}

static void netvar_table_parse(
    vector<netvar_info<native_data_map::field>> &storage, //
    native_data_map const *dmap, size_t const offset, bool const filter_duplicates)
{
    using netvar_info = netvar_info<native_data_map::field>;

    auto do_parse = [&storage, dmap, offset]<bool Duplicates>(std::bool_constant<Duplicates>) {
        auto const fields_end = dmap->fields + dmap->fields_count;
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
                    if (std::any_of(
                            storage.cbegin(), storage.cend(), //
                            _1->*&netvar_info::raw_name == field->name))
                        continue;
                }

                [[maybe_unused]] //
                auto &new_item = storage.emplace_back
#ifdef FD_MERGE_NETVAR_TABLES
                                 (field, offset);
#else
                                 (field);
#endif
#ifdef _DEBUG
                if constexpr (Duplicates)
                    move_last<true>(storage, new_item);
#endif
            }
        }
    };

    if (!filter_duplicates)
        do_parse(std::false_type());
    else
        do_parse(std::true_type());
}

template <class T>
struct netvar_table final : basic_netvar_table
{
    friend class netvar_storage;

    using pointer = T const *;

    using info_type    = netvar_info<typename T::value_type>;
    using storage_type = vector<info_type>;

  private:
    pointer source_;
    storage_type storage_;
#ifndef FD_MERGE_NETVAR_TABLES
    vector<netvar_table> inner_;
#endif

    void sort()
    {
        std::stable_sort(storage_.begin(), storage_.end(), _1->*&info_type::offset < _2->*&info_type::offset);
    }

  public:
    netvar_table(pointer const source)
        : source_(source)
    {
        netvar_table_parse(storage_, source, 0, false);
    }

    void parse(pointer item, size_t offset, bool filter_duplicates = false)
    {
        netvar_table_parse(storage_, item, offset, filter_duplicates);
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
        auto const end = storage_.end();
        auto const it  = std::find_if(storage_.begin(), end, _1->*&info_type::pretty_name == name);
        return it == end ? nullptr : iterator_to_raw_pointer(it);
    }

    char const *raw_name() const
    {
        return source_->name;
    }

    char const *name() const
    {
        return netvar_table_name(source_);
    }

    bool empty() const
    {
        return storage_.empty();
    }
};

class netvar_types_cache final : public basic_netvar_types_cache
{
    struct stored_value
    {
        key_type key;
        buffer_type type;

        string_view view() const
        {
            return {type.begin(), type.end()};
        }
    };

    vector<stored_value> storage_;

  public:
    string_view get(key_type key) const override
    {
        auto const end = storage_.end();
        auto const it  = std::find_if(storage_.begin(), end, _1->*&stored_value::key == key);
        return it == end ? string_view() : it->view();
    }

    string_view store(key_type key, buffer_type &&value) override
    {
        return storage_.emplace_back(key, move(value)).view();
    }
};

template <bool Unwrap = true, class T>
static auto find_helper(T &rng, auto fn)
{
    auto end = rng.end();
    auto it  = std::find_if(rng.begin(), end, fn);
    if constexpr (Unwrap)
        return it == end ? nullptr : iterator_to_raw_pointer(it);
    else
        return it;
}

static bool equal_name_compare(char const *ptr, char const *ptr2)
{
    return ptr == ptr2;
}

static bool equal_name_compare(char const *ptr, string_view const str)
{
    return string_view::traits_type::compare(ptr, str.data(), str.length()) == 0;
}

template <typename Fn, typename T>
class equal_name
{
    Fn fn_;
    T test_;

  public:
    equal_name(Fn fn, T test)
        : fn_(std::move(fn))
        , test_(std::move(test))
    {
    }

    bool operator()(auto &src) const
    {
        return equal_name_compare(std::invoke(fn_, src), test_);
    }
};

enum class netvar_tables_add_state
{
    nothing,
    unique,
    merge
};

template <class T>
class netvar_tables_storage
{
    using table_type = netvar_table<T>;

    using pointer       = table_type *;
    using const_pointer = table_type const *;

    using storage_type = vector<table_type>;

    vector<table_type> storage_;
    netvar_types_cache types_cache_;

    template <bool RawName>
    static constexpr auto name_fn()
    {
        if constexpr (RawName)
            return &table_type::raw_name;
        else
            return &table_type::name;
    }

  public:
    string_view type(typename table_type::info_type const *info)
    {
        return info->type(&types_cache_);
    }

    template <bool RawName = true>
    const_pointer get(string_view name) const
    {
        auto end   = storage_.end();
        auto found = std::find_if(storage_.begin(), end, equal_name(name_fn<RawName>(), name));
        return found == end ? nullptr : iterator_to_raw_pointer(found);
    }

    template <bool RawName = true>
    pointer get(string_view name)
    {
        return remove_const(as_const(this)->template get<RawName>(name));
    }

    template <bool RawName = true, typename N>
    bool contains(N name) const
    {
        return std::any_of(storage_.begin(), storage_.end(), equal_name(name_fn<RawName>(), name));
    }

    template <netvar_tables_add_state State>
    void add(typename table_type::pointer item)
    {
        using add_state = netvar_tables_add_state;
        if constexpr (State != add_state::unique)
            if constexpr (State == add_state::merge)
            {
                auto end         = storage_.end();
                auto const found = std::find_if(storage_.begin(), end, equal_name(name_fn<true>(), item->name));
                if (found != end)
                {
                    found->parse(item, 0, true);
                    return;
                }
            }
            else
            {
                assert(!contains<true>(item->name));
            }

        table_type tmp(item);
        if (!tmp.empty())
            storage_.emplace_back(std::move(tmp));
    }

    bool empty() const
    {
        return storage_.empty();
    }
};

class netvar_storage final : public basic_netvar_storage
{
    netvar_tables_storage<native_recv_table> recv_tables_;
    netvar_tables_storage<native_data_map> data_maps_;

  public:
    basic_netvar_table *get(string_view const name) override
    {
        if (name.starts_with(class_prefix))
            return data_maps_.get(name);
        if (name.starts_with(datamap_prefix))
            return recv_tables_.get(name);

#if 1
        using std::copy;
        using std::max;

        constexpr auto name_offset   = max(datamap_prefix.length(), class_prefix.length());
        constexpr auto class_start   = name_offset - class_prefix.length();
        constexpr auto datamap_start = name_offset - datamap_prefix.length();

        //....[prefix][name]....
        //^^reserved   ^^always on same place
        //   for long prefix
        array<char, 128> buff;
        auto const end = copy(name.begin(), name.end(), next(buff.begin(), name_offset));

        auto it = copy(class_prefix.begin(), class_prefix.end(), next(buff.begin(), class_start));
        if (auto const ptr = data_maps_.get({it, end}))
            return ptr;
        it = copy(datamap_prefix.begin(), datamap_prefix.end(), next(buff.begin(), datamap_start));
        if (auto const ptr = recv_tables_.get({it, end}))
            return ptr;
#else
        if (auto ptr = data_maps_.get<false>(name))
            return ptr;
        if (auto ptr = recv_tables_.get<false>(name))
            return ptr;
#endif
        return nullptr;
    }

    void store(native_client_class const *root) override
    {
        if (recv_tables_.empty())
        {
            for (; root != nullptr; root = root->next)
                recv_tables_.add<netvar_tables_add_state::unique>(root->table);
        }
        else
        {
            for (; root != nullptr; root = root->next)
                recv_tables_.add<netvar_tables_add_state::merge>(root->table);
        }
    }

    void store(native_data_map const *root) override
    {
        if (data_maps_.empty())
        {
            for (; root != nullptr; root = root->base)
                data_maps_.add<netvar_tables_add_state::unique>(root);
        }
        else
        {
            for (; root != nullptr; root = root->base)
                data_maps_.add<netvar_tables_add_state::merge>(root);
        }
    }

    void save(wstring_view directory) const override
    {
        // todo: sort if not debug
        ignore_unused(directory);
        unreachable();
    }

    void load(wstring_view directory, uint8_t version) override
    {
        ignore_unused(directory, version);
        unreachable();
    }
};

FD_OBJECT_IMPL(netvar_storage);
} // namespace fd