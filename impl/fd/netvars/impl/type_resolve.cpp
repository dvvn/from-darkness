#include "type_resolve.h"

#include <fd/valve/client_side/base_entity.h>
#include <fd/valve/color.h>
#include <fd/valve/entity_handle.h>
#include <fd/valve/matrix3x4.h>
#include <fd/valve/matrix4x4.h>
#include <fd/valve/qangle.h>
#include <fd/valve/quaternion.h>
#include <fd/valve/vector2d.h>
#include <fd/valve/vector3d.h>

#include <fmt/format.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <cctype>
#include <optional>

namespace fd
{

template <typename T>
static constexpr std::false_type _NetvarTypeFor;

#define NETVAR_TYPE_PLATFORM(_T_) \
    template <>                   \
    static constexpr auto _NetvarTypeFor<_T_> = platform_netvar_type(#_T_);

NETVAR_TYPE_PLATFORM(int8_t);
NETVAR_TYPE_PLATFORM(uint8_t);
NETVAR_TYPE_PLATFORM(int16_t);
NETVAR_TYPE_PLATFORM(uint16_t);
NETVAR_TYPE_PLATFORM(int32_t);
NETVAR_TYPE_PLATFORM(uint32_t);
NETVAR_TYPE_PLATFORM(int64_t);
NETVAR_TYPE_PLATFORM(uint64_t);

#define NETVAR_TYPE_NATIVE(_T_) \
    template <>                 \
    static constexpr auto _NetvarTypeFor<_T_> = native_netvar_type(#_T_);

NETVAR_TYPE_NATIVE(bool);
NETVAR_TYPE_NATIVE(char *);
NETVAR_TYPE_NATIVE(const char *);
NETVAR_TYPE_NATIVE(void *);
NETVAR_TYPE_NATIVE(const void *);
NETVAR_TYPE_NATIVE(float);
NETVAR_TYPE_NATIVE(double);
NETVAR_TYPE_NATIVE(long double);

#define VALVE_INCLUDE(_INC)           "<fd/valve/" BOOST_STRINGIZE(_INC_) ".h>"
#define VALVE_INCLUDE_EX(_EX_, _INC_) "<fd/valve/" BOOST_STRINGIZE(_EX_) "/" BOOST_STRINGIZE(_INC_) ".h>"

#define NETVAR_TYPE_VALVE(_T_, _INC_)                                             \
    template <>                                                                   \
    static constexpr auto _NetvarTypeFor<valve::_T_> = custom_netvar_type_simple( \
        BOOST_STRINGIZE(_T_), VALVE_INCLUDE(_INC_));

#define NETVAR_TYPE_VALVE_CLIENT(_T_, _INC_)                                                   \
    template <>                                                                                \
    static constexpr auto _NetvarTypeFor<valve::client_side::_T_> = custom_netvar_type_simple( \
        BOOST_STRINGIZE(_T_), VALVE_INCLUDE_EX(client_side, _INC_));

// NETVAR_TYPE_VALVE(vector, vector);
NETVAR_TYPE_VALVE(vector2d, vector2d);
NETVAR_TYPE_VALVE(vector3d, vector3d);
NETVAR_TYPE_VALVE(color, color);
NETVAR_TYPE_VALVE(qangle, qangle);
NETVAR_TYPE_VALVE(handle, entity_handle);
NETVAR_TYPE_VALVE_CLIENT(base_entity *, base_entity);
NETVAR_TYPE_VALVE(quaternion, quaternion);
// NETVAR_TYPE_VALVE(view_matrix, matrixX);
NETVAR_TYPE_VALVE(matrix3x4, matrix3x4);
NETVAR_TYPE_VALVE(matrix4x4, matrix4x4);

// template <>
// static constexpr auto _NetvarTypeFor<valve::handle> =
//     custom_netvar_type<std::string_view, std::array<std::string_view, 2>>(
//         "handle",
//         { VALVE_INCLUDE(entity_handle), VALVE_INCLUDE_EX(client_side, entity_list) });

//-------------

static constexpr std::string_view internal_prefix = ("m_");

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
            return { start, up };
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
            return { start, up };
#else
        auto end = type.end();
        for (auto it = start; it != end; ++it)
        {
            if (isupper(*it))
                return { start, it };
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

static std::optional<custom_netvar_type_simple> _check_int_prefix(std::string_view type)
{
    if (_check_prefix(type, "uch"))
        return _NetvarTypeFor<valve::color>;
    return {};
}

static std::optional<custom_netvar_type_simple> _check_float_prefix(std::string_view type)
{
#if 1
    if (type == "m_rgflCoordinateFrame")
        __debugbreak();
    auto prefix = _find_exact_prefix(type, (3));
    if (prefix == "ang")
        return _NetvarTypeFor<valve::qangle>;
    if (prefix == "vec")
        return _NetvarTypeFor<valve::vector3d>;
    return {};
#else
    switch (_extract_prefix(type).substr(3))
    {
    case "ang"_hash:
        return _NetvarTypeFor<qangle>;
    case "vec"_hash:
        return _NetvarTypeFor<vector3d>;
    default:
        return {};
    }
#endif
}

static netvar_type _extract_type_integer(std::string_view name)
{
    auto prefix = _find_prefix(name, (3));
    switch (prefix.size())
    {
    case 1: {
        if (prefix[0] == 'b')
            return _NetvarTypeFor<bool>;
        if (prefix[0] == 'c')
            return _NetvarTypeFor<uint8_t>;
        if (prefix[0] == 'h')
            return _NetvarTypeFor<valve::handle>;
        break;
    }
    case 2: {
        if (prefix == "un")
            return _NetvarTypeFor<uint32_t>;
        if (prefix == "ch")
            return _NetvarTypeFor<uint8_t>;
        if (prefix == "fl" && name.ends_with("Time")) //  SimulationTime int ???
            return _NetvarTypeFor<float>;
        break;
    }
    case 3: {
        if (prefix == "clr")
            return _NetvarTypeFor<valve::color>; // not sure
        break;
    }
    };

    return _NetvarTypeFor<int32_t>;
}

static netvar_type _extract_type_vec3(std::string_view name)
{
    assert(!std::isdigit(name[0]));

    constexpr auto &qang = _NetvarTypeFor<valve::qangle>;
    constexpr auto &vec  = _NetvarTypeFor<valve::vector3d>;

    if (_check_prefix(name, "ang"))
        return qang;
    auto real_name = name.substr(std::strlen("m_***"));
    if (real_name.size() >= std::strlen("angles"))
    {
        auto second_real_name_char = real_name.find("ngles");
        if (second_real_name_char != real_name.npos && second_real_name_char != 0)
        {
            auto first_real_name_char = real_name[second_real_name_char - 1];
            if (first_real_name_char == 'a' || first_real_name_char == 'A')
                return qang;
        }
    }
    return vec;
}

static netvar_type _extract_type(std::string_view name, valve::recv_prop *prop)
{
    using pt = valve::recv_prop_type;

    switch (prop->type)
    {
    case pt::DPT_Int:
        return _extract_type_integer(name);
    case pt::DPT_Float:
        return _NetvarTypeFor<float>;
    case pt::DPT_Vector:
        return _extract_type_vec3(name);
    case pt::DPT_VectorXY:
        return _NetvarTypeFor<valve::vector2d>; // 3d vector. z unused
    case pt::DPT_String:
        return _NetvarTypeFor<char *>; // char[X]
    case pt::DPT_Array: {
        auto prev_prop = prop - 1;
        // assert(std::string_view(prevProp->name).ends_with("[0]"));
        return netvar_type_array(prop->elements_count, _extract_type(name, prev_prop));
    }
    case pt::DPT_DataTable: {
#if 0
        return prop->name;
#else
        assert(0 && "Data table type must be manually resolved!");
        return _NetvarTypeFor<void *>;
#endif
    }
    case pt::DPT_Int64:
        return _NetvarTypeFor<int64_t>;
    default: {
        assert(0 && "Unknown recv prop type");
        return _NetvarTypeFor<void *>;
    }
    }
}

static netvar_type _extract_type(std::string_view name, valve::data_map_description *field)
{
    using ft = valve::data_map_description_type;

    switch (field->type)
    {
    case ft::FIELD_VOID:
        return _NetvarTypeFor<void *>;
    case ft::FIELD_FLOAT:
        return _NetvarTypeFor<float>;
    case ft::FIELD_STRING:
        return _NetvarTypeFor<char *>; // string_t at real
    case ft::FIELD_VECTOR:
        return _extract_type_vec3(name);
    case ft::FIELD_QUATERNION:
        return _NetvarTypeFor<valve::quaternion>;
    case ft::FIELD_INTEGER:
        return _extract_type_integer(name);
    case ft::FIELD_BOOLEAN:
        return _NetvarTypeFor<bool>;
    case ft::FIELD_SHORT:
        return _NetvarTypeFor<int16_t>;
    case ft::FIELD_CHARACTER:
        return _NetvarTypeFor<int8_t>;
    case ft::FIELD_COLOR32:
        return _NetvarTypeFor<valve::color>;
    case ft::FIELD_EMBEDDED:
        assert(0 && "Embedded field detected");
        std::unreachable();
    case ft::FIELD_CUSTOM:
        assert(0 && "Custom field detected");
        std::unreachable();
    case ft::FIELD_CLASSPTR:
        return _NetvarTypeFor<valve::client_side::base_entity *>;
    case ft::FIELD_EHANDLE:
        return _NetvarTypeFor<valve::handle>;
    case ft::FIELD_EDICT:
        assert(0 && "Edict field detected"); //  "edict_t*"
        std::unreachable();
    case ft::FIELD_POSITION_VECTOR:
        return _NetvarTypeFor<valve::vector3d>;
    case ft::FIELD_TIME:
        return _NetvarTypeFor<float>;
    case ft::FIELD_TICK:
        return _NetvarTypeFor<int32_t>;
    case ft::FIELD_MODELNAME:
    case ft::FIELD_SOUNDNAME:
        return _NetvarTypeFor<char *>; // string_t at real
    case ft::FIELD_INPUT:
        assert(0 && "Inputvar field detected"); //  "CMultiInputVar"
        std::unreachable();
    case ft::FIELD_FUNCTION:
        assert(0 && "Function detected");
        std::unreachable();
    case ft::FIELD_VMATRIX:
    case ft::FIELD_VMATRIX_WORLDSPACE:
        return _NetvarTypeFor<valve::matrix4x4>; // VMatrix
    case ft::FIELD_MATRIX3X4_WORLDSPACE:
        return _NetvarTypeFor<valve::matrix3x4>;
    case ft::FIELD_INTERVAL:
        assert(0 && "Interval field detected"); // "interval_t"
        std::unreachable();
    case ft::FIELD_MODELINDEX:
    case ft::FIELD_MATERIALINDEX:
        return _NetvarTypeFor<int32_t>;
    case ft::FIELD_VECTOR2D:
        return _NetvarTypeFor<valve::vector2d>;
    default:
        assert(0 && "Unknown datamap field type");
        std::unreachable();
    }
}

static std::optional<custom_netvar_type_simple> _extract_type_by_prefix(std::string_view name, valve::recv_prop *prop)
{
    using pt = valve::recv_prop_type;

    switch (prop->type)
    {
    case pt::DPT_Int:
        return _check_int_prefix(name);
    case pt::DPT_Float:
        return _check_float_prefix(name);
    default:
        return {};
    }
}

static std::optional<custom_netvar_type_simple> _extract_type_by_prefix(
    std::string_view name,
    valve::data_map_description *field)
{
    using ft = valve::data_map_description_type;

    switch (field->type)
    {
    case ft::FIELD_INTEGER:
        return _check_int_prefix(name);
    case ft::FIELD_FLOAT:
        return _check_float_prefix(name);
    default:
        return {};
    }
}

netvar_type netvar_type_hint::resolve() const
{
    return std::visit(
        [&]<typename T>(T val) -> netvar_type {
            if constexpr (std::same_as<std::monostate, T>)
                std::terminate();
            else
            {
                if (arraySize <= 1)
                    return _extract_type(name, val);
                if (arraySize == 3)
                {
                    auto type = _extract_type_by_prefix(name, val);
                    if (type.has_value())
                        return *std::move(type);
                }
                return netvar_type_array(arraySize, _extract_type(name, val));
            }
        },
        source);
}
} // namespace fd