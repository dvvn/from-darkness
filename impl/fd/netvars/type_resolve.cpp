#include <fd/netvars/type_resolve.h>

#include <fmt/format.h>

#include <cassert>
#include <cctype>

namespace fd
{
static char const* _prefix_ptr(char const* ptr, size_t prefixSize)
{
    if (!std::isupper(ptr[2 + prefixSize]))
        return nullptr;
    if (*ptr++ != 'm' || *ptr++ != '_')
        return nullptr;
    return ptr;
}

static bool _check_prefix(std::string_view type, std::string_view prefix)
{
    if (type.size() - 2 <= prefix.size())
        return false;
    auto ptr = _prefix_ptr(type.data(), prefix.size());
    return ptr && std::memcmp(ptr, prefix.data(), prefix.size()) == 0;
}

[[maybe_unused]]
static bool _check_prefix(std::string_view type, char const prefix)
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

static std::string_view _find_prefix(
    std::string_view         type,
    const _prefix_max_length limit = std::numeric_limits<uint16_t>::max())
{
    if (!type.starts_with("m_"))
        return {};
    for (size_t i = 2; i < std::min(limit.value, type.size()); ++i)
    {
        if (!std::isupper(type[i]))
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

static std::string_view _find_prefix(std::string_view type, const _prefix_length prefixLength)
{
    if (!type.starts_with("m_"))
        return {};
    if (type.size() - 2 <= prefixLength.value)
        return {};
    if (!std::isupper(type[2 + prefixLength.value]))
        return {};

    auto prefix = type.substr(2, prefixLength.value);
    for (auto c : prefix)
    {
        if (std::isupper(c))
            return {};
    }
    return prefix;
}

static std::string_view _check_int_prefix(std::string_view type)
{
    if (_check_prefix(type, "uch"))
        return "valve::color";
    return {};
}

static std::string_view _check_float_prefix(std::string_view type)
{
#if 1
    auto prefix = _find_prefix(type, _prefix_length(3));
    if (prefix == "ang")
        return "valve::qangle";
    if (prefix == "vec")
        return "valve::vector3";
    return {};
#else
    switch (_extract_prefix(type).substr(3))
    {
    case "ang"_hash:
        return "qangle";
    case "vec"_hash:
        return "vector3";
    default:
        return {};
    }
#endif
}

#ifndef __cpp_lib_string_contains
#define contains(_X_) offset_to(_X_) != static_cast<size_t>(-1)
#endif

struct not_explicit_string : std::string
{
    template <typename... Args>
    not_explicit_string(Args&&... args)
        : std::string(std::forward<Args>(args)...)
    {
    }
};

static not_explicit_string _type_recv_prop(std::string_view name, valve::recv_prop* prop)
{
    using pt = valve::recv_prop_type;

    switch (prop->type)
    {
    case pt::DPT_Int:
        return extract_type_integer(name);
    case pt::DPT_Float:
        return "float";
    case pt::DPT_Vector:
        return extract_type_vec3(name);
    case pt::DPT_VectorXY:
        return "valve::vector2"; // 3d vector. z unused
    case pt::DPT_String:
        return "char*"; // char[X]
    case pt::DPT_Array:
    {
        auto prevProp = prop - 1;
        //assert(std::string_view(prevProp->name).ends_with("[0]"));
        auto type = extract_type(name, prevProp);
        return extract_type_std_array(type, prop->elements_count);
    }
    case pt::DPT_DataTable:
    {
#if 0
        return prop->name;
#else
        assert(0 && "Data table type must be manually resolved!");
        return "void*";
#endif
    }
    case pt::DPT_Int64:
        return "int64_t";
    default:
    {
        assert(0 && "Unknown recv prop type");
        return "void*";
    }
    }
}

std::string extract_type_std_array(std::string_view type, size_t size)
{
    assert(size != 0);
    return fmt::format("std::array<{}, {}>", type, size);
}

std::string extract_type_valve_vector(std::string_view type)
{
    return fmt::format("valve::vector<{}>", type);
}

std::string_view extract_type_vec3(std::string_view name)
{
    assert(!std::isdigit(name[0]));

    constexpr auto qang = "valve::qangle";
    constexpr auto vec  = "valve::vector3";

    if (_check_prefix(name, "ang"))
        return qang;
    auto netvarName = name.substr(std::strlen("m_***"));
    if (netvarName.size() >= std::strlen("angles"))
    {
        auto anglesWordPos = netvarName.find("ngles");
        if (anglesWordPos != netvarName.npos && anglesWordPos > 0)
        {
            auto anglesWordBegin = netvarName[anglesWordPos - 1];
            if (anglesWordBegin == 'a' || anglesWordBegin == 'A')
                return qang;
        }
    }
    return vec;
}

static bool operator==(std::string_view str, char const c)
{
    return str[0] == c;
}

std::string_view extract_type_integer(std::string_view name)
{
    auto prefix = _find_prefix(name, _prefix_max_length(3));
    switch (prefix.size())
    {
    case 1:
    {
        if (prefix == 'b')
            return "bool";
        if (prefix == 'c')
            return "uint8_t";
        if (prefix == 'h')
            return "valve::base_handle";
        break;
    }
    case 2:
    {
        if (prefix == "un")
            return "uint32_t";
        if (prefix == "ch")
            return "uint8_t";
        if (prefix == "fl" && name.ends_with("Time")) //  SimulationTime int ???
            return "float";
        break;
    }
    case 3:
    {
        if (prefix == "clr")
            return "valve::color"; // not sure
        break;
    }
    };

    return "int32_t";
}

//---

std::string extract_type(std::string_view name, valve::recv_prop* prop)
{
    return _type_recv_prop(name, prop);
}

std::string_view extract_type(std::string_view name, valve::data_map_description* field)
{
    using ft = valve::data_map_description_type;

    switch (field->type)
    {
    case ft::FIELD_VOID:
        return "void*";
    case ft::FIELD_FLOAT:
        return "float";
    case ft::FIELD_STRING:
        return "char*"; // string_t at real
    case ft::FIELD_VECTOR:
        return extract_type_vec3(name);
    case ft::FIELD_QUATERNION:
        return "valve::quaternion";
    case ft::FIELD_INTEGER:
        return extract_type_integer(name);
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
        return "valve::base_entity*";
    case ft::FIELD_EHANDLE:
        return "valve::base_handle";
    case ft::FIELD_EDICT:
        assert(0 && "Edict field detected"); //  "edict_t*"
        std::unreachable();
    case ft::FIELD_POSITION_VECTOR:
        return "valve::vector3";
    case ft::FIELD_TIME:
        return "float";
    case ft::FIELD_TICK:
        return "int32_t";
    case ft::FIELD_MODELNAME:
    case ft::FIELD_SOUNDNAME:
        return "char*"; // string_t at real
    case ft::FIELD_INPUT:
        assert(0 && "Inputvar field detected"); //  "CMultiInputVar"
        std::unreachable();
    case ft::FIELD_FUNCTION:
        assert(0 && "Function detected");
        std::unreachable();
    case ft::FIELD_VMATRIX:
    case ft::FIELD_VMATRIX_WORLDSPACE:
        return "valve::view_matrix";
    case ft::FIELD_MATRIX3X4_WORLDSPACE:
        return "valve::matrix3x4";
    case ft::FIELD_INTERVAL:
        assert(0 && "Interval field detected"); // "interval_t"
        std::unreachable();
    case ft::FIELD_MODELINDEX:
    case ft::FIELD_MATERIALINDEX:
        return "int32_t";
    case ft::FIELD_VECTOR2D:
        return "valve::vector2";
    default:
        assert(0 && "Unknown datamap field type");
        std::unreachable();
    }
}

std::string_view extract_type_by_prefix(std::string_view name, valve::recv_prop* prop)
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

std::string_view extract_type_by_prefix(std::string_view name, valve::data_map_description* field)
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