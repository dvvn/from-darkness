#include "source.h"
#include "type.h"

#include <fd/valve/data_map.h>
#include <fd/valve/recv_table.h>

#include <boost/unordered/unordered_flat_map.hpp>

#include <algorithm>
#include <cctype>
#include <memory>
#include <utility>

namespace fd
{
namespace types_cache
{
template <typename V>
using map_type = boost::unordered::unordered_flat_map<std::string_view, V>;

struct type_stored : std::unique_ptr<basic_netvar_type>
{
    template <typename T, typename... Args>
    type_stored(std::in_place_type_t<T>, Args &&...args)
        : std::unique_ptr<basic_netvar_type>(new T(std::forward<Args>(args)...))
    {
    }
};

static map_type<type_stored> cache;
static map_type<std::string> cloned_names;

//----------

class valve_name
{
    char buff_[32];
    uint8_t length_;

  public:
    consteval valve_name(auto &name)
        : buff_()
        , length_(0)
    {
        auto append = [this](auto &val) {
            std::copy(std::begin(val), std::end(val) - 1, buff_);
            length_ += std::size(val) - 1;
        };

        append("valve::");
        append(name);
    }

    operator std::string_view() const
    {
        return {buff_, length_};
    }
};

static std::string_view clone_key(std::string_view key, _const<char *> gap)
{
    auto result = cloned_names.try_emplace(key);
    auto &str   = result.first->second;
    if (result.second)
    {
        assert(gap != nullptr);
        auto gap_length = strlen(gap);
        str.reserve(key.size() + gap_length);
        str.append(key).append(gap, gap + gap_length);
    }
    else
    {
        assert(str.ends_with(gap));
    }
    return str;
}

template <typename T, typename... Args>
static basic_netvar_type *try_get(std::string_view key, Args &&...args)
{
    return cache.try_emplace(key, std::in_place_type<T>, std::forward<Args>(args)...).first->second.get();
}

static basic_netvar_type *try_get(std::string_view key)
{
    auto it = cache.find(key);
    if (it == cache.end())
        return nullptr;
    return it->second.get();
}

static basic_netvar_type *simple(std::string_view key, _const<valve_name &> name)
{
    return try_get<netvar_type<std::string>, std::string_view>(key, name);
}

static basic_netvar_type *trivial(std::string_view key, auto &name)
{
    constexpr auto name_length = sizeof(name) - 1;
    assert(strlen(name) == name_length);
    return try_get<netvar_type<char[name_length]>>(key, name);
}

template <typename S>
using array_fn = basic_netvar_type *(std::string_view, S *);

template <class T>
class array_fn_holder
{
    T fn_;

  public:
    array_fn_holder(T fn)
        : fn_(std::move(fn))
    {
    }

    operator std::string_view()
    {
        return fn_()->get();
    }
};

template <typename S>
static auto array_fn_wrapper(array_fn<S> *fn, std::string_view arg, S *source)
{
    return array_fn_holder(std::bind(fn, arg, source));
}

template <typename S>
static basic_netvar_type *array(std::string_view key, array_fn<S> *fn, S *source, size_t length)
{
    return try_get<netvar_type_array>(clone_key(key, "_arr"), array_fn_wrapper(fn, key, source), length);
}

static basic_netvar_type *array(std::string_view key, basic_netvar_type *inner, size_t length)
{
    return try_get<netvar_type_array>(clone_key(key, "_arr"), inner->get(), length);
}
} // namespace types_cache

#define READ_CACHE(_SRC_)                                       \
    if (auto ptr = types_cache::try_get(_SRC_); ptr != nullptr) \
    {                                                           \
        return ptr;                                             \
    }

using valve::data_map_description;
using valve::recv_prop;

class prefix_string
{
    static constexpr std::string_view internal_prefix = "m_";
    std::string_view type_;

  public:
    prefix_string(std::string_view type)
        : type_((type))
    {
    }

    operator std::string_view() const
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
            return std::isupper(tmp.back()) && islower(tmp.front());
        default:
            return std::isupper(tmp[length]) && std::ranges::all_of(tmp.substr(0, length), islower);
        }
    }

    bool contains(std::string_view prefix) const
    {
        assert(operator bool());

        auto tmp = type_.substr(internal_prefix.length());
        if (tmp.length() <= prefix.length() + 1)
            return false;
        if (!isupper(tmp[prefix.length()]))
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

    std::string_view extract(size_t max_length = std::numeric_limits<size_t>::max() - 1) const
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

    std::string_view extract_exact(size_t length) const
    {
        assert(operator bool());

        auto start   = type_.data() + internal_prefix.size();
        auto abs_end = type_.data() + type_.size();
        auto end     = type_.data() + length + 1;

        return {start, end < abs_end && contains(length) ? end : start};
    }
};

static basic_netvar_type *_check_int_prefix(prefix_string name)
{
    if (name)
    {
        READ_CACHE(name);

        if (name.contains("uch"))
            return types_cache::simple(name, "color");
    }
    return nullptr;
}

static basic_netvar_type *_check_float_prefix(prefix_string name)
{
    if (name)
    {
#ifdef _DEBUG
        if (name.contains("rgfl"))
        {
            __debugbreak();
            return nullptr;
        }
#endif

        READ_CACHE(name);

        auto prefix = name.extract_exact(3);
        if (prefix == "ang")
            return types_cache::simple(name, "qangle");
        if (prefix == "vec")
            return types_cache::simple(name, "vector3d");
    }
    return nullptr;
}

static basic_netvar_type *_extract_type_integer(prefix_string name)
{
    if (name)
    {
        READ_CACHE(name);

        auto prefix = name.extract(3);
        switch (prefix.size())
        {
        case 1: {
            if (prefix[0] == 'b')
                return types_cache::trivial(name, "bool");
            if (prefix[0] == 'c')
                return types_cache::trivial(name, "uint8_t");
            if (prefix[0] == 'h')
                return types_cache::simple(name, "handle");
            break;
        }
        case 2: {
            if (prefix == "un")
                return types_cache::trivial(name, "uint32_t");
            if (prefix == "ch")
                return types_cache::trivial(name, "uint8_t");
            if (prefix == "fl" && std::string_view(name).ends_with("Time")) //  SimulationTime int ???
                return types_cache::trivial(name, "float");
            break;
        }
        case 3: {
            if (prefix == "clr")
                return types_cache::simple(name, "color"); // not sure
            break;
        }
        };
    }
    return types_cache::trivial(name, "int32_t");
}

static basic_netvar_type *_extract_type_vec3(prefix_string name)
{
    if (name)
    {
        READ_CACHE(name);

        auto is_qangle = [name = std::string_view(name)] {
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
            return types_cache::simple(name, "qangle");
    }

    return types_cache::simple(name, "vector3d");
}

static basic_netvar_type *_extract_type(std::string_view name, recv_prop *prop)
{
    using pt = valve::recv_prop_type;

    switch (prop->type)
    {
    case pt::DPT_Int:
        return _extract_type_integer(name);
    case pt::DPT_Float:
        return types_cache::trivial(name, "float");
    case pt::DPT_Vector:
        return _extract_type_vec3(name);
    case pt::DPT_VectorXY:
        return types_cache::simple(name, "vector2d"); // 3d vector. z unused
    case pt::DPT_String:
        return types_cache::trivial(name, "char *"); // char[X]
    case pt::DPT_Array: {
        auto prev_prop = prop - 1;
        return types_cache::array<recv_prop>(name, _extract_type, prev_prop, prop->elements_count);
    }
    case pt::DPT_DataTable: {
#if 0
        return prop->name;
#else
        assert(0 && "Data table type must be manually resolved!");
        return types_cache::trivial(name, "void *");
#endif
    }
    case pt::DPT_Int64:
        return types_cache::trivial(name, "int64_t");
    default: {
        assert(0 && "Unknown recv prop type");
        return types_cache::trivial(name, "void *");
    }
    }
}

static basic_netvar_type *_extract_type(std::string_view name, data_map_description *field)
{
    using ft = valve::data_map_description_type;

    switch (field->type)
    {
    case ft::FIELD_VOID:
        return types_cache::trivial(name, "void *");
    case ft::FIELD_FLOAT:
        return types_cache::trivial(name, "float");
    case ft::FIELD_STRING:
        return types_cache::trivial(name, "char *"); // string_t at real
    case ft::FIELD_VECTOR:
        return _extract_type_vec3(name);
    case ft::FIELD_QUATERNION:
        return types_cache::simple(name, "quaternion");
    case ft::FIELD_INTEGER:
        return _extract_type_integer(name);
    case ft::FIELD_BOOLEAN:
        return types_cache::trivial(name, "bool");
    case ft::FIELD_SHORT:
        return types_cache::trivial(name, "int16_t");
    case ft::FIELD_CHARACTER:
        return types_cache::trivial(name, "int8_t");
    case ft::FIELD_COLOR32:
        return types_cache::simple(name, "color");
    case ft::FIELD_EMBEDDED:
        assert(0 && "Embedded field detected");
        std::unreachable();
    case ft::FIELD_CUSTOM:
        assert(0 && "Custom field detected");
        std::unreachable();
    case ft::FIELD_CLASSPTR:
        return /*types_cache::simple(name,client_side::base_entity *)*/ types_cache::trivial(name, "void *");
    case ft::FIELD_EHANDLE:
        return types_cache::simple(name, "handle");
    case ft::FIELD_EDICT:
        assert(0 && "Edict field detected"); //  "edict_t*"
        std::unreachable();
    case ft::FIELD_POSITION_VECTOR:
        return types_cache::simple(name, "vector3d");
    case ft::FIELD_TIME:
        return types_cache::trivial(name, "float");
    case ft::FIELD_TICK:
        return types_cache::trivial(name, "int32_t");
    case ft::FIELD_MODELNAME:
    case ft::FIELD_SOUNDNAME:
        return types_cache::trivial(name, "char *"); // string_t at real
    case ft::FIELD_INPUT:
        assert(0 && "Inputvar field detected"); //  "CMultiInputVar"
        std::unreachable();
    case ft::FIELD_FUNCTION:
        assert(0 && "Function detected");
        std::unreachable();
    case ft::FIELD_VMATRIX:
    case ft::FIELD_VMATRIX_WORLDSPACE:
        return types_cache::simple(name, "matrix4x4"); // VMatrix
    case ft::FIELD_MATRIX3X4_WORLDSPACE:
        return types_cache::simple(name, "matrix3x4");
    case ft::FIELD_INTERVAL:
        assert(0 && "Interval field detected"); // "interval_t"
        std::unreachable();
    case ft::FIELD_MODELINDEX:
    case ft::FIELD_MATERIALINDEX:
        return types_cache::trivial(name, "int32_t");
    case ft::FIELD_VECTOR2D:
        return types_cache::simple(name, "vector2d");
    default:
        assert(0 && "Unknown datamap field type");
        std::unreachable();
    }
}

static basic_netvar_type *_extract_type_by_prefix(std::string_view name, recv_prop *prop)
{
    using pt = valve::recv_prop_type;

    switch (prop->type)
    {
    case pt::DPT_Int:
        return _check_int_prefix(name);
    case pt::DPT_Float:
        return _check_float_prefix(name);
    default:
        return nullptr;
    }
}

static basic_netvar_type *_extract_type_by_prefix(std::string_view name, data_map_description *field)
{
    using ft = valve::data_map_description_type;

    switch (field->type)
    {
    case ft::FIELD_INTEGER:
        return _check_int_prefix(name);
    case ft::FIELD_FLOAT:
        return _check_float_prefix(name);
    default:
        return nullptr;
    }
}

netvar_source::netvar_source(recv_prop *pointer)
    : netvar_source(pointer, source::recv_prop)
{
}

netvar_source::netvar_source(data_map_description *pointer)
    : netvar_source(pointer, source::data_map)
{
}

netvar_source::netvar_source(void *pointer, source src)
    : pointer_(pointer)
    , src_(src)
{
}

template <typename Ret, typename T>
static Ret extract(void *pointer, Ret T::*value)
{
    return std::invoke(value, static_cast<T *>(pointer));
}

template <typename T, typename Fn>
static auto extract(void *pointer, Fn fn)
{
    return std::invoke(fn, static_cast<T *>(pointer));
}

_const<char *> netvar_source::name() const
{
    switch (src_)
    {
    case source::recv_prop:
        return extract(pointer_, &recv_prop::name);
    case source::data_map:
        return extract(pointer_, &data_map_description::name);
    default:
        std::unreachable();
    }
}

size_t netvar_source::offset() const
{
    switch (src_)
    {
    case source::recv_prop:
        return extract(pointer_, &recv_prop::offset);
    case source::data_map:
        return extract(pointer_, &data_map_description::offset);
    default:
        std::unreachable();
    }
}

basic_netvar_type *netvar_source::type(std::string_view correct_name, size_t array_size) const
{
    auto do_extract = [&]<class T>(T *p) {
        if (correct_name.empty())
            correct_name = p->name;

        basic_netvar_type *type = nullptr;
        if (array_size <= 1)
            type = _extract_type(correct_name, p);
        else if (array_size == 3)
            type = _extract_type_by_prefix(correct_name, p);

        if (!type)
        {
            type = types_cache::array<T>(correct_name, _extract_type, p, array_size);
        }

        return type;
    };

    switch (src_)
    {
    case source::recv_prop:
        return extract<recv_prop>(pointer_, do_extract);
    case source::data_map:
        return extract<data_map_description>(pointer_, do_extract);
    default:
        std::unreachable();
    }
}

bool netvar_source::operator==(_const<netvar_source &> other) const
{
    return pointer_ == other.pointer_;
}
} // namespace fd