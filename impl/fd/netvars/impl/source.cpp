#include "source.h"
#include "type.h"

#include <fd/valve2/data_map.h>
#include <fd/valve2/recv_table.h>

#include <boost/static_string.hpp>
#include <boost/unordered/unordered_flat_map.hpp>
#include <boost/unordered/unordered_flat_set.hpp>

#include <algorithm>
#include <cctype>
#include <memory>
#include <utility>

using boost::container::flat_map;
using boost::unordered::unordered_flat_map;
using boost::unordered::unordered_flat_set;

namespace fd
{
using valve::data_map_description;
using valve::recv_prop;

static constexpr std::string_view internal_prefix = "m_";

static bool is_valid_prefix(std::string_view type, size_t whole_prefix_size)
{
    return type.size() > whole_prefix_size &&       //
           std::isupper(type[whole_prefix_size]) && //
           type.starts_with(internal_prefix);
}

static bool _check_prefix(std::string_view type, std::string_view prefix)
{
    auto whole_prefix_size = internal_prefix.size() + prefix.size();
    return is_valid_prefix(type, whole_prefix_size) && type.substr(internal_prefix.size()).starts_with(prefix);
}

static bool can_have_prefix(std::string_view type)
{
    // m_xX
    if (type.size() < internal_prefix.size() + 2)
        return false;
    if (!type.starts_with(internal_prefix))
        return false;
    return true;
}

static std::string_view _find_prefix(std::string_view type)
{
    if (can_have_prefix(type))
    {
        auto start = type.begin() + internal_prefix.size();
        auto end   = type.end();

        auto up = std::find_if(start, end, isupper);
        if (up != end)
            return {start, up};
    }
    return {};
}

static std::string_view _find_prefix(std::string_view type, size_t max_length)
{
    if (can_have_prefix(type))
    {
        auto start = type.begin() + internal_prefix.size();
#if 1
        auto end = start + std::min(max_length + 1, type.size() - internal_prefix.size());

        auto up = std::find_if(start, end, isupper);
        if (up != end)
            return {start, up};
#else
        auto end = type.end();
        for (auto it = start; it != end; ++it)
        {
            if (isupper(*it))
                return {start, it};
            if (max_length == 0)
                break;
            --max_length;
        }
#endif
    }
    return {};
}

static std::string_view _find_exact_prefix(std::string_view type, size_t length)
{
    auto whole_prefix_size = internal_prefix.size() + length;
    if (!is_valid_prefix(type, whole_prefix_size))
        return {};
    auto prefix = type.substr(internal_prefix.size(), length);
    if (std::any_of(prefix.begin(), prefix.end(), isupper))
        return {};
    return prefix;
}

static class
{
    using key_type = std::string_view;

    struct value_type : std::unique_ptr<basic_netvar_type>
    {
        template <typename T, typename... Args>
        value_type(std::in_place_type_t<T>, Args &&...args)
            : std::unique_ptr<basic_netvar_type>(new T(std::forward<Args>(args)...))
        {
        }
    };

    unordered_flat_map<key_type, value_type> cache_;
    unordered_flat_map<std::string_view, std::string> name_clone_;

    class valve_name
    {
        boost::static_string<64> buff_;

      public:
        template <size_t S>
        consteval valve_name(char const (&name)[S])
        {
            buff_.append("valve::", 7).append(name, S - 1);
        }

        operator std::string_view() const
        {
            return {buff_.begin(), buff_.end()};
        }
    };

    static auto get_data(std::string_view str)
    {
        return str.data();
    }

    static auto get_data(char const *str)
    {
        return str;
    }

    template <typename T = std::false_type>
    key_type clone_key(key_type key, T &&gap = {})
    {
        auto result = name_clone_.try_emplace(key);
        auto &str   = result.first->second;
        if (result.second)
        {
            if constexpr (std::constructible_from<key_type, T &&>)
            {
                auto gap1 = key_type(std::forward<T>(gap));
                str.reserve(key.size() + gap1.size());
                str.append(key).append(gap1);
            }
            else
            {
                assert(0 && "key not found");
            }
        }
        else
        {
            if constexpr (std::constructible_from<key_type, T &&>)
            {
                assert(str.ends_with(gap));
            }
        }
        return str;
    }

  public:
    template <typename T, typename... Args>
    basic_netvar_type *try_get(key_type key, Args &&...args)
    {
        return cache_.try_emplace(key, std::in_place_type<T>, std::forward<Args>(args)...).first->second.get();
    }

    basic_netvar_type *try_get(key_type key) const
    {
        auto it = cache_.find(key);
        if (it == cache_.end())
            return nullptr;
        return it->second.get();
    }

    basic_netvar_type *simple(key_type key, valve_name name)
    {
        return try_get<simple_netvar_type>(key, name);
    }

    template <size_t S>
    basic_netvar_type *trivial(key_type key, char const (&name)[S])
    {
        return try_get<simple_netvar_type>(key, name);
    }

    template <typename S>
    basic_netvar_type *array(key_type key, basic_netvar_type *(*fn)(std::string_view, S *), S *source, size_t length)
    {
        struct wrapper
        {
            basic_netvar_type *(*fn)(std::string_view, S *);
            key_type key;
            S *source;

            operator std::string_view() const
            {
                return fn(key, source)->get();
            }
        };

        return try_get<netvar_type_array>(clone_key(key, "_arr"), wrapper(fn, key, source), length);
    }

    basic_netvar_type *array(key_type key, basic_netvar_type *inner, size_t length)
    {
        return try_get<netvar_type_array>(clone_key(key, "_arr"), inner->get(), length);
    }
} types_cache;

#define READ_CACHE(_SRC_)                                      \
    if (auto ptr = types_cache.try_get(_SRC_); ptr != nullptr) \
    {                                                          \
        return ptr;                                            \
    }

static basic_netvar_type *_check_int_prefix(std::string_view name)
{
    READ_CACHE(name);

    if (_check_prefix(name, "uch"))
        return types_cache.simple(name, "color");
    return nullptr;
}

static basic_netvar_type *_check_float_prefix(std::string_view name)
{
    READ_CACHE(name);

#if 1
    if (name == "m_rgflCoordinateFrame")
        __debugbreak();
    auto prefix = _find_exact_prefix(name, 3);
    if (prefix == "ang")
        return types_cache.simple(name, "qangle");
    if (prefix == "vec")
        return types_cache.simple(name, "vector3d");
    return nullptr;
#else
    switch (_extract_prefix(type).substr(3))
    {
    case "ang"_hash:
        return SIMPLE_TYPE(qangle);
    case "vec"_hash:
        return SIMPLE_TYPE(vector3d);
    default:
        return 0;
    }
#endif
}

static basic_netvar_type *_extract_type_integer(std::string_view name)
{
    READ_CACHE(name);

    auto prefix = _find_prefix(name, 3);
    switch (prefix.size())
    {
    case 1: {
        if (prefix[0] == 'b')
            return types_cache.trivial(name, "bool");
        if (prefix[0] == 'c')
            return types_cache.trivial(name, "uint8_t");
        if (prefix[0] == 'h')
            return types_cache.simple(name, "handle");
        break;
    }
    case 2: {
        if (prefix == "un")
            return types_cache.trivial(name, "uint32_t");
        if (prefix == "ch")
            return types_cache.trivial(name, "uint8_t");
        if (prefix == "fl" && name.ends_with("Time")) //  SimulationTime int ???
            return types_cache.trivial(name, "float");
        break;
    }
    case 3: {
        if (prefix == "clr")
            return types_cache.simple(name, "color"); // not sure
        break;
    }
    };

    return types_cache.trivial(name, "int32_t");
}

static basic_netvar_type *_extract_type_vec3(std::string_view name)
{
    READ_CACHE(name);

    assert(!std::isdigit(name[0]));

    auto is_qangle = [&] {
        if (_check_prefix(name, "ang"))
            return true;
        auto real_name = name.substr(std::strlen("m_***"));
        if (real_name.size() >= std::strlen("angles"))
        {
            auto second_real_name_char = real_name.find("ngles");
            if (second_real_name_char != real_name.npos && second_real_name_char != 0)
            {
                auto first_real_name_char = real_name[second_real_name_char - 1];
                if (first_real_name_char == 'a' || first_real_name_char == 'A')
                    return true;
            }
        }
        return false;
    };

    return is_qangle() ? types_cache.simple(name, "qangle") : types_cache.simple(name, "vector3d");
}

static basic_netvar_type *_extract_type(std::string_view name, recv_prop *prop)
{
    using pt = valve::recv_prop_type;

    switch (prop->type)
    {
    case pt::DPT_Int:
        return _extract_type_integer(name);
    case pt::DPT_Float:
        return types_cache.trivial(name, "float");
    case pt::DPT_Vector:
        return _extract_type_vec3(name);
    case pt::DPT_VectorXY:
        return types_cache.simple(name, "vector2d"); // 3d vector. z unused
    case pt::DPT_String:
        return types_cache.trivial(name, "char *"); // char[X]
    case pt::DPT_Array: {
        auto prev_prop = prop - 1;
        return types_cache.array<recv_prop>(name, _extract_type, prev_prop, prop->elements_count);
    }
    case pt::DPT_DataTable: {
#if 0
        return prop->name;
#else
        assert(0 && "Data table type must be manually resolved!");
        return types_cache.trivial(name, "void *");
#endif
    }
    case pt::DPT_Int64:
        return types_cache.trivial(name, "int64_t");
    default: {
        assert(0 && "Unknown recv prop type");
        return types_cache.trivial(name, "void *");
    }
    }
}

static basic_netvar_type *_extract_type(std::string_view name, data_map_description *field)
{
    using ft = valve::data_map_description_type;

    switch (field->type)
    {
    case ft::FIELD_VOID:
        return types_cache.trivial(name, "void *");
    case ft::FIELD_FLOAT:
        return types_cache.trivial(name, "float");
    case ft::FIELD_STRING:
        return types_cache.trivial(name, "char *"); // string_t at real
    case ft::FIELD_VECTOR:
        return _extract_type_vec3(name);
    case ft::FIELD_QUATERNION:
        return types_cache.simple(name, "quaternion");
    case ft::FIELD_INTEGER:
        return _extract_type_integer(name);
    case ft::FIELD_BOOLEAN:
        return types_cache.trivial(name, "bool");
    case ft::FIELD_SHORT:
        return types_cache.trivial(name, "int16_t");
    case ft::FIELD_CHARACTER:
        return types_cache.trivial(name, "int8_t");
    case ft::FIELD_COLOR32:
        return types_cache.simple(name, "color");
    case ft::FIELD_EMBEDDED:
        assert(0 && "Embedded field detected");
        std::unreachable();
    case ft::FIELD_CUSTOM:
        assert(0 && "Custom field detected");
        std::unreachable();
    case ft::FIELD_CLASSPTR:
        return /*types_cache.simple(name,client_side::base_entity *)*/ types_cache.trivial(name, "void *");
    case ft::FIELD_EHANDLE:
        return types_cache.simple(name, "handle");
    case ft::FIELD_EDICT:
        assert(0 && "Edict field detected"); //  "edict_t*"
        std::unreachable();
    case ft::FIELD_POSITION_VECTOR:
        return types_cache.simple(name, "vector3d");
    case ft::FIELD_TIME:
        return types_cache.trivial(name, "float");
    case ft::FIELD_TICK:
        return types_cache.trivial(name, "int32_t");
    case ft::FIELD_MODELNAME:
    case ft::FIELD_SOUNDNAME:
        return types_cache.trivial(name, "char *"); // string_t at real
    case ft::FIELD_INPUT:
        assert(0 && "Inputvar field detected"); //  "CMultiInputVar"
        std::unreachable();
    case ft::FIELD_FUNCTION:
        assert(0 && "Function detected");
        std::unreachable();
    case ft::FIELD_VMATRIX:
    case ft::FIELD_VMATRIX_WORLDSPACE:
        return types_cache.simple(name, "matrix4x4"); // VMatrix
    case ft::FIELD_MATRIX3X4_WORLDSPACE:
        return types_cache.simple(name, "matrix3x4");
    case ft::FIELD_INTERVAL:
        assert(0 && "Interval field detected"); // "interval_t"
        std::unreachable();
    case ft::FIELD_MODELINDEX:
    case ft::FIELD_MATERIALINDEX:
        return types_cache.trivial(name, "int32_t");
    case ft::FIELD_VECTOR2D:
        return types_cache.simple(name, "vector2d");
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

char const *netvar_source::name() const
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
            type = types_cache.array<T>(correct_name, _extract_type, p, array_size);
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
} // namespace fd