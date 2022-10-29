﻿module;

#include <fd/assert.h>

#include <array>
#include <span>

module fd.netvars.type_resolve;
import fd.math.color;
import fd.math.qangle;
import fd.math.view_matrix;
import fd.math.vector2;
import fd.valve.vector;
import fd.valve.base_handle;
import fd.string.info;
import fd.type_name;
import fd.string.make;

namespace fd::valve
{
    class base_entity;
}

using namespace fd;
using namespace valve;

string netvars::type_std_array(const string_view type, const size_t size)
{
    FD_ASSERT(size != 0);
    constexpr auto arr_name = type_name<std::array>();
#if defined(__cpp_lib_format) && 0
    return std::format("{}<{}, {}>", arr_name, type, size); // formatter not imported
#else
    return make_string(arr_name, '<', type, ", ", size, '>');
#endif
}

string netvars::type_utlvector(const string_view type)
{
    constexpr auto arr_name = type_name<valve::vector>();
#if defined(__cpp_lib_format) && 0
    return std::format("{}<{}>", arr_name, type); // formatter not imported
#else
    return make_string(arr_name, '<', type, '>');
#endif
}

// m_xxxX***
static string_view _Extract_prefix(const string_view type, const size_t prefix_size = 3)
{
    const auto type_start = 2 + prefix_size;
    if (type.size() <= type_start)
        return {};
    if (/* type.substr(0, 2) != "m_" */ type[0] != 'm' || type[1] != '_')
        return {};
    if (!is_upper(type[type_start]))
        return {};
    return type.substr(2, prefix_size);
}

#ifndef __cpp_lib_string_contains
#define contains(_X_) find(_X_) != static_cast<size_t>(-1)
#endif

string_view netvars::type_vec3(const string_view type)
{
    const auto vec3_qangle = [=] {
        if (is_digit(type[0]))
            return false;
        const auto prefix = _Extract_prefix(type);
        if (prefix == "ang")
            return true;
        const auto type_fixed = prefix.empty() ? type : type.substr(2);
        const auto ang        = type_fixed.find("ngles");
        if (ang == type_fixed.npos || ang == 0)
            return false;
        const auto chr = type_fixed[ang - 1];
        return chr == 'a' || chr == 'A';
    };

    return vec3_qangle() ? type_name<math::qangle>() : type_name<math::vector3>();
}

string_view netvars::type_integer(string_view type)
{
    if (/*!is_digit(type[0]) &&*/ type.starts_with("m_"))
    {
        type.remove_prefix(2);
        const auto check_upper = [&](const size_t i) {
            return type.size() > i && is_upper(type[i]);
        };
        if (check_upper(1))
        {
            if (type.starts_with('b'))
                return type_name<bool>();
            if (type.starts_with('c'))
                return type_name<uint8_t>();
            if (type.starts_with('h'))
                return type_name<base_handle>();
        }
        else if (check_upper(2))
        {
            if (type.starts_with("un"))
                return type_name<uint32_t>();
            if (type.starts_with("ch"))
                return type_name<uint8_t>();
            if (type.starts_with("fl") && /* to_lower */ type.substr(2).ends_with("Time") /* contains("time") */) //  SimulationTime int ???
                return type_name<float>();
        }
        else if (check_upper(3))
        {
            if (type.starts_with("clr"))
                return type_name<math::color>(); // not sure
        }
    }

    return type_name<int32_t>();
}

//---

string netvars::type_recv_prop(const recv_prop* prop)
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
        return type_name<math::vector2>(); // 3d vector. z unused
    case pt::DPT_String:
        return type_name<char*>(); // char[X]
    case pt::DPT_Array: {
        const auto prev_prop = prop - 1;
        FD_ASSERT(string_view(prev_prop->name).ends_with("[0]"));
        const auto type = type_recv_prop(prev_prop);
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

string_view netvars::type_datamap_field(const data_map_description* field)
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
        return type_name<math::quaternion>();
    case ft::FIELD_INTEGER:
        return type_integer(field->name);
    case ft::FIELD_BOOLEAN:
        return type_name<bool>();
    case ft::FIELD_SHORT:
        return type_name<int16_t>();
    case ft::FIELD_CHARACTER:
        return type_name<int8_t>();
    case ft::FIELD_COLOR32:
        return type_name<math::color>();
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
        return type_name<math::vector3>();
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
        return type_name<math::view_matrix>();
    case ft::FIELD_MATRIX3X4_WORLDSPACE:
        return type_name<math::matrix3x4>();
    case ft::FIELD_INTERVAL:
        FD_ASSERT_UNREACHABLE("Interval field detected"); // "interval_t"
    case ft::FIELD_MODELINDEX:
    case ft::FIELD_MATERIALINDEX:
        return type_name<int32_t>();
    case ft::FIELD_VECTOR2D:
        return type_name<math::vector2>();
    default:
        FD_ASSERT_UNREACHABLE("Unknown datamap field type");
    }
}

static string_view _Check_int_prefix(const string_view type)
{
    if (_Extract_prefix(type) == "uch")
        return type_name<math::color>();
    return {};
}

static string_view _Check_float_prefix(const string_view type)
{
#if 1
    const auto prefix = _Extract_prefix(type);
    if (prefix == "ang")
        return type_name<math::qangle>();
    if (prefix == "vec")
        return type_name<math::vector3>();
    return {};
#else
    switch (_Extract_prefix(type).substr(3))
    {
    case "ang"_hash:
        return type_name<math::qangle>();
    case "vec"_hash:
        return type_name<math::vector3>();
    default:
        return {};
    }
#endif
}

string_view netvars::type_array_prefix(const string_view type, const recv_prop* prop)
{
    using pt = recv_prop_type;

    switch (prop->type)
    {
    case pt::DPT_Int:
        return _Check_int_prefix(type);
    case pt::DPT_Float:
        return _Check_float_prefix(type);
    default:
        return {};
    }
}

string_view netvars::type_array_prefix(const string_view type, const data_map_description* field)
{
    using ft = data_map_description_type;

    switch (field->type)
    {
    case ft::FIELD_INTEGER:
        return _Check_int_prefix(type);
    case ft::FIELD_FLOAT:
        return _Check_float_prefix(type);
    default:
        return {};
    }
}