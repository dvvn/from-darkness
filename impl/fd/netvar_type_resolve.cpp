#include <fd/assert.h>
#include <fd/netvar_type_resolve.h>
#include <fd/string_info.h>
#include <fd/type_name.h>

// ReSharper disable CppUnusedIncludeDirective
#include <fd/valve/base_entity.h>
#include <fd/valve/base_handle.h>
#include <fd/valve/color.h>
#include <fd/valve/qangle.h>
#include <fd/valve/quaternion.h>
#include <fd/valve/vector.h>
#include <fd/valve/vectorX.h>

#include <array>
#include <span>

using namespace fd;
using namespace valve;

// m_xxxX***
static string_view _extract_prefix(const string_view type, const size_t prefixSize = 3)
{
    const auto typeStart = 2 + prefixSize;
    if (type.size() <= typeStart)
        return {};
    if (/* type.substr(0, 2) != "m_" */ !(type[0] == 'm' && type[1] == '_'))
        return {};
    if (!is_upper(type[typeStart]))
        return {};
    return type.substr(2, prefixSize);
}

static string_view _check_int_prefix(const string_view type)
{
    if (_extract_prefix(type) == "uch")
        return type_name<color>();
    return {};
}

static string_view _check_float_prefix(const string_view type)
{
#if 1
    const auto prefix = _extract_prefix(type);
    if (prefix == "ang")
        return type_name<qangle>();
    if (prefix == "vec")
        return type_name<vector3>();
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
#define contains(_X_) find(_X_) != static_cast<size_t>(-1)
#endif

struct unexpl_string : string
{
    template <typename... Args>
    unexpl_string(Args&&... args)
        : string(std::forward<Args>(args)...)
    {
    }
};

static unexpl_string _type_recv_prop(const recv_prop* prop)
{
    using pt = recv_prop_type;

    switch (prop->type)
    {
    case pt::DPT_Int:
        return type_integer(prop->name);
    case pt::DPT_Float:
        return type_name<float>();
    case pt::DPT_Vector:
        return type_vec3(prop->name);
    case pt::DPT_VectorXY:
        return type_name<vector2>(); // 3d vector. z unused
    case pt::DPT_String:
        return type_name<char*>(); // char[X]
    case pt::DPT_Array: {
        const auto prevProp = prop - 1;
        FD_ASSERT(string_view(prevProp->name).ends_with("[0]"));
        const auto type = type_recv_prop(prevProp);
        return type_std_array(type, prop->elements_count);
    }
    case pt::DPT_DataTable: {
#if 1
        return prop->name;
#else
        FD_ASSERT("Data table type must be manually resolved!");
        return type_name<void*>();
#endif
    }
    case pt::DPT_Int64:
        return type_name<int64_t>();
    default: {
        FD_ASSERT_UNREACHABLE("Unknown recv prop type");
    }
    }
}

namespace fd
{
    string type_std_array(const string_view type, const size_t size)
    {
        FD_ASSERT(size != 0);
        constexpr auto arrName = /*type_name<std::array>()*/ "std::array";
#if defined(__cpp_lib_format) && 0
        return std::format("{}<{}, {}>", arr_name, type, size); // formatter not imported
#else
        return make_string(arrName, '<', type, ", ", size, '>');
#endif
    }

    string type_utlvector(const string_view type)
    {
        constexpr auto arrName = /*type_name<valve::vector>()*/ "valve::vector";
#if defined(__cpp_lib_format) && 0
        return std::format("{}<{}>", arr_name, type); // formatter not imported
#else
        return make_string(arrName, '<', type, '>');
#endif
    }

    string_view type_vec3(const string_view type)
    {
        const auto vec3Qangle = [=] {
            if (is_digit(type[0]))
                return false;
            const auto prefix = _extract_prefix(type);
            if (prefix == "ang")
                return true;
            const auto typeFixed = prefix.empty() ? type : type.substr(2);
            const auto ang       = typeFixed.find("ngles");
            if (ang == typeFixed.npos || ang == 0)
                return false;
            const auto chr = typeFixed[ang - 1];
            return chr == 'a' || chr == 'A';
        }();

        return vec3Qangle ? type_name<qangle>() : type_name<vector3>();
    }

    string_view type_integer(string_view type)
    {
        if (/*!is_digit(type[0]) &&*/ type.starts_with("m_"))
        {
            type.remove_prefix(2);
            const auto checkUpper = [&](const size_t i) {
                return type.size() > i && is_upper(type[i]);
            };
            if (checkUpper(1))
            {
                if (type.starts_with('b'))
                    return type_name<bool>();
                if (type.starts_with('c'))
                    return type_name<uint8_t>();
                if (type.starts_with('h'))
                    return type_name<base_handle>();
            }
            else if (checkUpper(2))
            {
                if (type.starts_with("un"))
                    return type_name<uint32_t>();
                if (type.starts_with("ch"))
                    return type_name<uint8_t>();
                if (type.starts_with("fl") && /* to_lower */ type.substr(2).ends_with("Time") /* contains("time") */) //  SimulationTime int ???
                    return type_name<float>();
            }
            else if (checkUpper(3))
            {
                if (type.starts_with("clr"))
                    return type_name<color>(); // not sure
            }
        }

        return type_name<int32_t>();
    }

    //---

    string type_recv_prop(const recv_prop* prop)
    {
        return _type_recv_prop(prop);
    }

    string_view type_datamap_field(const data_map_description* field)
    {
        using ft = data_map_description_type;

        switch (field->type)
        {
        case ft::FIELD_VOID:
            return type_name<void*>();
        case ft::FIELD_FLOAT:
            return type_name<float>();
        case ft::FIELD_STRING:
            return type_name<char*>(); // string_t at real
        case ft::FIELD_VECTOR:
            return type_vec3(field->name);
        case ft::FIELD_QUATERNION:
            return type_name<quaternion>();
        case ft::FIELD_INTEGER:
            return type_integer(field->name);
        case ft::FIELD_BOOLEAN:
            return type_name<bool>();
        case ft::FIELD_SHORT:
            return type_name<int16_t>();
        case ft::FIELD_CHARACTER:
            return type_name<int8_t>();
        case ft::FIELD_COLOR32:
            return type_name<color>();
        case ft::FIELD_EMBEDDED:
            FD_ASSERT_UNREACHABLE("Embedded field detected");
        case ft::FIELD_CUSTOM:
            FD_ASSERT_UNREACHABLE("Custom field detected");
        case ft::FIELD_CLASSPTR:
            return type_name<base_entity*>();
        case ft::FIELD_EHANDLE:
            return type_name<base_handle>();
        case ft::FIELD_EDICT:
            FD_ASSERT_UNREACHABLE("Edict field detected"); //  "edict_t*"
        case ft::FIELD_POSITION_VECTOR:
            return type_name<vector3>();
        case ft::FIELD_TIME:
            return type_name<float>();
        case ft::FIELD_TICK:
            return type_name<int32_t>();
        case ft::FIELD_MODELNAME:
        case ft::FIELD_SOUNDNAME:
            return type_name<char*>(); // string_t at real
        case ft::FIELD_INPUT:
            FD_ASSERT_UNREACHABLE("Inputvar field detected"); //  "CMultiInputVar"
        case ft::FIELD_FUNCTION:
            FD_ASSERT_UNREACHABLE("Function detected");
        case ft::FIELD_VMATRIX:
        case ft::FIELD_VMATRIX_WORLDSPACE:
            return type_name<view_matrix>();
        case ft::FIELD_MATRIX3X4_WORLDSPACE:
            return type_name<matrix3x4>();
        case ft::FIELD_INTERVAL:
            FD_ASSERT_UNREACHABLE("Interval field detected"); // "interval_t"
        case ft::FIELD_MODELINDEX:
        case ft::FIELD_MATERIALINDEX:
            return type_name<int32_t>();
        case ft::FIELD_VECTOR2D:
            return type_name<vector2>();
        default:
            FD_ASSERT_UNREACHABLE("Unknown datamap field type");
        }
    }

    string_view type_array_prefix(const string_view type, const recv_prop* prop)
    {
        using pt = recv_prop_type;

        switch (prop->type)
        {
        case pt::DPT_Int:
            return _check_int_prefix(type);
        case pt::DPT_Float:
            return _check_float_prefix(type);
        default:
            return {};
        }
    }

    string_view type_array_prefix(const string_view type, const data_map_description* field)
    {
        using ft = data_map_description_type;

        switch (field->type)
        {
        case ft::FIELD_INTEGER:
            return _check_int_prefix(type);
        case ft::FIELD_FLOAT:
            return _check_float_prefix(type);
        default:
            return {};
        }
    }
} // namespace fd