#include <fd/assert.h>
#include <fd/format.h>
#include <fd/netvar_type_resolve.h>
#include <fd/string_info.h>
#include <fd/type_name.h>

#include <fd/valve/base_entity.h>
#include <fd/valve/base_handle.h>
#include <fd/valve/color.h>
#include <fd/valve/cs_player.h>
#include <fd/valve/qangle.h>
#include <fd/valve/quaternion.h>
#include <fd/valve/vector.h>
#include <fd/valve/vectorX.h>

namespace fd
{
FD_TYPE_NAME_PRECACHE(valve::qangle);
FD_TYPE_NAME_PRECACHE(valve::vector2);
FD_TYPE_NAME_PRECACHE(valve::vector3);
FD_TYPE_NAME_PRECACHE(valve::vector4);
FD_TYPE_NAME_PRECACHE(valve::matrix3x4);
FD_TYPE_NAME_PRECACHE(valve::view_matrix); // matrix4x4
FD_TYPE_NAME_PRECACHE(valve::base_handle);
FD_TYPE_NAME_PRECACHE(valve::base_entity*);
FD_TYPE_NAME_PRECACHE(valve::cs_player*);
FD_TYPE_NAME_PRECACHE(valve::color);
FD_TYPE_NAME_PRECACHE(valve::quaternion);

static const char* _prefix_ptr(const char* ptr, const size_t prefixSize)
{
    if (!is_upper(ptr[2 + prefixSize]))
        return nullptr;
    if (*ptr++ != 'm' || *ptr++ != '_')
        return nullptr;
    return ptr;
}

static bool _check_prefix(const string_view type, const string_view prefix)
{
    if (type.size() - 2 <= prefix.size())
        return false;
    auto ptr = _prefix_ptr(type.data(), prefix.size());
    return ptr && equal(prefix, ptr);
}

[[maybe_unused]] static bool _check_prefix(const string_view type, const char prefix)
{
    if (type.size() <= 3)
        return false;
    auto ptr = _prefix_ptr(type.data(), 1);
    return ptr && *ptr == prefix;
}

struct _prefix_max_length
{
    constexpr _prefix_max_length(size_t value)
        : value(value)
    {
    }

    size_t value;
};

static string_view _find_prefix(const string_view type, const _prefix_max_length limit = std::numeric_limits<uint16_t>::max())
{
    if (!type.starts_with("m_"))
        return {};
    for (size_t i = 2; i < std::min(limit.value, type.size()); ++i)
    {
        if (!is_upper(type[i]))
            continue;
        return type.substr(2, i);
    }
    return {};
}

struct _prefix_length
{
    constexpr _prefix_length(size_t value)
        : value(value)
    {
    }

    size_t value;
};

static string_view _find_prefix(const string_view type, const _prefix_length prefixLength)
{
    if (!type.starts_with("m_"))
        return {};
    if (type.size() - 2 <= prefixLength.value)
        return {};
    if (!is_upper(type[2 + prefixLength.value]))
        return {};

    const auto prefix = type.substr(2, prefixLength.value);
    for (const auto c : prefix)
    {
        if (is_upper(c))
            return {};
    }
    return prefix;
}

static string_view _check_int_prefix(const string_view type)
{
    if (_check_prefix(type, "uch"))
        return type_name<valve::color>();
    return {};
}

static string_view _check_float_prefix(const string_view type)
{
#if 1
    const auto prefix = _find_prefix(type, _prefix_length(3));
    if (prefix == "ang")
        return type_name<valve::qangle>();
    if (prefix == "vec")
        return type_name<valve::vector3>();
    return {};
#else
    switch (_extract_prefix(type).substr(3))
    {
    case "ang"_hash:
        return type_name<qangle>();
    case "vec"_hash:
        return type_name<vector3>();
    default:
        return {};
    }
#endif
}

#ifndef __cpp_lib_string_contains
#define contains(_X_) offset_to(_X_) != static_cast<size_t>(-1)
#endif

struct not_explicit_string : string
{
    template <typename... Args>
    not_explicit_string(Args&&... args)
        : string(std::forward<Args>(args)...)
    {
    }
};

static not_explicit_string _type_recv_prop(string_view name, const valve::recv_prop* prop)
{
    using pt = valve::recv_prop_type;

    switch (prop->type)
    {
    case pt::DPT_Int:
        return extract_type_integer(name);
    case pt::DPT_Float:
        return type_name<float>();
    case pt::DPT_Vector:
        return extract_type_vec3(name);
    case pt::DPT_VectorXY:
        return type_name<valve::vector2>(); // 3d vector. z unused
    case pt::DPT_String:
        return type_name<char*>(); // char[X]
    case pt::DPT_Array: {
        const auto prevProp = prop - 1;
        FD_ASSERT(string_view(prevProp->name).ends_with("[0]"));
        const auto type = extract_type(name, prevProp);
        return extract_type_std_array(type, prop->elements_count);
    }
    case pt::DPT_DataTable: {
#if 0
        return prop->name;
#else
        FD_ASSERT("Data table type must be manually resolved!");
        return type_name<void*>();
#endif
    }
    case pt::DPT_Int64:
        return type_name<int64_t>();
    default: {
        FD_ASSERT_PANIC("Unknown recv prop type");
    }
    }
}

string extract_type_std_array(const string_view type, const size_t size)
{
    FD_ASSERT(size != 0);
    constexpr auto arrName = /*type_name<std::array>()*/ "std::array";
#if defined(__cpp_lib_format) && 0
    return std::format("{}<{}, {}>", arr_name, type, size); // formatter not imported
#else
    return make_string(arrName, '<', type, ", ", to_string(size), '>');
#endif
}

string extract_type_valve_vector(const string_view type)
{
    constexpr auto arrName = /*type_name<valve::vector>()*/ "valve::vector";
#if defined(__cpp_lib_format) && 0
    return std::format("{}<{}>", arr_name, type); // formatter not imported
#else
    return make_string(arrName, '<', type, '>');
#endif
}

string_view extract_type_vec3(const string_view name)
{
    FD_ASSERT(!is_digit(name[0]));

    constexpr auto qang = type_name<valve::qangle>();
    constexpr auto vec  = type_name<valve::vector3>();

    if (_check_prefix(name, "ang"))
        return qang;
    const auto netvarName = name.substr(str_len("m_***"));
    if (netvarName.size() >= str_len("angles"))
    {
        const auto anglesWordPos = netvarName.find("ngles");
        if (anglesWordPos != netvarName.npos && anglesWordPos > 0)
        {
            const auto anglesWordBegin = netvarName[anglesWordPos - 1];
            if (anglesWordBegin == 'a' || anglesWordBegin == 'A')
                return qang;
        }
    }
    return vec;
}

static bool operator==(const string_view str, const char c)
{
    return str[0] == c;
}

string_view extract_type_integer(const string_view name)
{
    const auto prefix = _find_prefix(name, _prefix_max_length(3));
    switch (prefix.size())
    {
    case 1: {
        if (prefix == 'b')
            return type_name<bool>();
        if (prefix == 'c')
            return type_name<uint8_t>();
        if (prefix == 'h')
            return type_name<valve::base_handle>();
        break;
    }
    case 2: {
        if (prefix == "un")
            return type_name<uint32_t>();
        if (prefix == "ch")
            return type_name<uint8_t>();
        if (prefix == "fl" && name.ends_with("Time")) //  SimulationTime int ???
            return type_name<float>();
        break;
    }
    case 3: {
        if (prefix == "clr")
            return type_name<valve::color>(); // not sure
        break;
    }
    };

    return type_name<int32_t>();
}

//---

string extract_type(const string_view name, const valve::recv_prop* prop)
{
    return _type_recv_prop(name, prop);
}

string_view extract_type(const string_view name, const valve::data_map_description* field)
{
    using ft = valve::data_map_description_type;

    switch (field->type)
    {
    case ft::FIELD_VOID:
        return type_name<void*>();
    case ft::FIELD_FLOAT:
        return type_name<float>();
    case ft::FIELD_STRING:
        return type_name<char*>(); // string_t at real
    case ft::FIELD_VECTOR:
        return extract_type_vec3(name);
    case ft::FIELD_QUATERNION:
        return type_name<valve::quaternion>();
    case ft::FIELD_INTEGER:
        return extract_type_integer(name);
    case ft::FIELD_BOOLEAN:
        return type_name<bool>();
    case ft::FIELD_SHORT:
        return type_name<int16_t>();
    case ft::FIELD_CHARACTER:
        return type_name<int8_t>();
    case ft::FIELD_COLOR32:
        return type_name<valve::color>();
    case ft::FIELD_EMBEDDED:
        FD_ASSERT_PANIC("Embedded field detected");
    case ft::FIELD_CUSTOM:
        FD_ASSERT_PANIC("Custom field detected");
    case ft::FIELD_CLASSPTR:
        return type_name<valve::base_entity*>();
    case ft::FIELD_EHANDLE:
        return type_name<valve::base_handle>();
    case ft::FIELD_EDICT:
        FD_ASSERT_PANIC("Edict field detected"); //  "edict_t*"
    case ft::FIELD_POSITION_VECTOR:
        return type_name<valve::vector3>();
    case ft::FIELD_TIME:
        return type_name<float>();
    case ft::FIELD_TICK:
        return type_name<int32_t>();
    case ft::FIELD_MODELNAME:
    case ft::FIELD_SOUNDNAME:
        return type_name<char*>(); // string_t at real
    case ft::FIELD_INPUT:
        FD_ASSERT_PANIC("Inputvar field detected"); //  "CMultiInputVar"
    case ft::FIELD_FUNCTION:
        FD_ASSERT_PANIC("Function detected");
    case ft::FIELD_VMATRIX:
    case ft::FIELD_VMATRIX_WORLDSPACE:
        return type_name<valve::view_matrix>();
    case ft::FIELD_MATRIX3X4_WORLDSPACE:
        return type_name<valve::matrix3x4>();
    case ft::FIELD_INTERVAL:
        FD_ASSERT_PANIC("Interval field detected"); // "interval_t"
    case ft::FIELD_MODELINDEX:
    case ft::FIELD_MATERIALINDEX:
        return type_name<int32_t>();
    case ft::FIELD_VECTOR2D:
        return type_name<valve::vector2>();
    default:
        FD_ASSERT_PANIC("Unknown datamap field type");
    }
}

string_view extract_type_by_prefix(const string_view name, const valve::recv_prop* prop)
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

string_view extract_type_by_prefix(const string_view name, const valve::data_map_description* field)
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
} // namespace fd