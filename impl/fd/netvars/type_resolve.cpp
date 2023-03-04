#include <fd/netvars/type_resolve.h>

#include <fd/valve/base_entity.h>
#include <fd/valve/base_handle.h>
#include <fd/valve/color.h>
#include <fd/valve/matrixX.h>
#include <fd/valve/qangle.h>
#include <fd/valve/quaternion.h>
#include <fd/valve/vector.h>
#include <fd/valve/vectorX.h>

#include <fmt/format.h>

#include <cassert>
#include <cctype>
#include <optional>

namespace fd
{
template <typename T>
static constexpr auto _NetvarTypeFor = nullptr;

#define NETVAR_TYPE_PLATFORM(_T_) \
    template <>                   \
    constexpr platform_netvar_type _NetvarTypeFor<_T_> = { #_T_ };

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
    constexpr native_netvar_type _NetvarTypeFor<_T_> = { #_T_ };

NETVAR_TYPE_NATIVE(bool);
NETVAR_TYPE_NATIVE(char*);
NETVAR_TYPE_NATIVE(const char*);
NETVAR_TYPE_NATIVE(void*);
NETVAR_TYPE_NATIVE(const void*);
NETVAR_TYPE_NATIVE(float);
NETVAR_TYPE_NATIVE(double);
NETVAR_TYPE_NATIVE(long double);

#define NETVAR_TYPE_VALVE(_T_, _INC_) \
    template <>                       \
    constexpr custom_netvar_type_simple _NetvarTypeFor<valve::_T_> = { "valve::" #_T_, "<fd/valve/" #_INC_ ".h>" };

// NETVAR_TYPE_VALVE(vector, vector);
NETVAR_TYPE_VALVE(vector2, vectorX);
NETVAR_TYPE_VALVE(vector3, vectorX);
NETVAR_TYPE_VALVE(color, color);
NETVAR_TYPE_VALVE(qangle, qangle);
NETVAR_TYPE_VALVE(base_handle, base_handle);
NETVAR_TYPE_VALVE(base_entity*, base_entity);
NETVAR_TYPE_VALVE(quaternion, quaternion);
// NETVAR_TYPE_VALVE(view_matrix, matrixX);
NETVAR_TYPE_VALVE(matrix3x4, matrixX);
NETVAR_TYPE_VALVE(matrix4x4, matrixX);

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
static bool _check_prefix(std::string_view type, char prefix)
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
    std::string_view   type,
    _prefix_max_length limit = std::numeric_limits<uint16_t>::max())
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

static std::string_view _find_prefix(std::string_view type, _prefix_length prefixLength)
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

static std::optional<custom_netvar_type_simple> _check_int_prefix(std::string_view type)
{
    if (_check_prefix(type, "uch"))
        return (_NetvarTypeFor<valve::color>);
    return {};
}

static std::optional<custom_netvar_type_simple> _check_float_prefix(std::string_view type)
{
#if 1
    auto prefix = _find_prefix(type, _prefix_length(3));
    if (prefix == "ang")
        return _NetvarTypeFor<valve::qangle>;
    if (prefix == "vec")
        return _NetvarTypeFor<valve::vector3>;
    return {};
#else
    switch (_extract_prefix(type).substr(3))
    {
    case "ang"_hash:
        return _NetvarTypeFor<qangle>;
    case "vec"_hash:
        return _NetvarTypeFor<vector3>;
    default:
        return {};
    }
#endif
}

static bool operator==(std::string_view str, char c)
{
    return str[0] == c;
}

template <typename Ret = known_netvar_type>
static Ret _extract_type_integer(std::string_view name)
{
    auto prefix = _find_prefix(name, _prefix_max_length(3));
    switch (prefix.size())
    {
    case 1:
    {
        if (prefix == 'b')
            return _NetvarTypeFor<bool>;
        if (prefix == 'c')
            return _NetvarTypeFor<uint8_t>;
        if (prefix == 'h')
            return _NetvarTypeFor<valve::base_handle>;
        break;
    }
    case 2:
    {
        if (prefix == "un")
            return _NetvarTypeFor<uint32_t>;
        if (prefix == "ch")
            return _NetvarTypeFor<uint8_t>;
        if (prefix == "fl" && name.ends_with("Time")) //  SimulationTime int ???
            return _NetvarTypeFor<float>;
        break;
    }
    case 3:
    {
        if (prefix == "clr")
            return _NetvarTypeFor<valve::color>; // not sure
        break;
    }
    };

    return _NetvarTypeFor<int32_t>;
}

template <typename Ret = known_netvar_type>
static Ret _extract_type_vec3(std::string_view name)
{
    assert(!std::isdigit(name[0]));

    constexpr auto qang = _NetvarTypeFor<valve::qangle>;
    constexpr auto vec  = _NetvarTypeFor<valve::vector3>;

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

static netvar_type _extract_type(std::string_view name, valve::recv_prop* prop)
{
    using pt = valve::recv_prop_type;

    switch (prop->type)
    {
    case pt::DPT_Int:
        return _extract_type_integer<netvar_type>(name);
    case pt::DPT_Float:
        return _NetvarTypeFor<float>;
    case pt::DPT_Vector:
        return _extract_type_vec3<netvar_type>(name);
    case pt::DPT_VectorXY:
        return _NetvarTypeFor<valve::vector2>; // 3d vector. z unused
    case pt::DPT_String:
        return _NetvarTypeFor<char*>; // char[X]
    case pt::DPT_Array:
    {
        auto prevProp = prop - 1;
        // assert(std::string_view(prevProp->name).ends_with("[0]"));
        return netvar_type_array(prop->elements_count, _extract_type(name, prevProp));
    }
    case pt::DPT_DataTable:
    {
#if 0
        return prop->name;
#else
        assert(0 && "Data table type must be manually resolved!");
        return _NetvarTypeFor<void*>;
#endif
    }
    case pt::DPT_Int64:
        return _NetvarTypeFor<int64_t>;
    default:
    {
        assert(0 && "Unknown recv prop type");
        return _NetvarTypeFor<void*>;
    }
    }
}

static netvar_type _extract_type(std::string_view name, valve::data_map_description* field)
{
    using ft = valve::data_map_description_type;

    switch (field->type)
    {
    case ft::FIELD_VOID:
        return _NetvarTypeFor<void*>;
    case ft::FIELD_FLOAT:
        return _NetvarTypeFor<float>;
    case ft::FIELD_STRING:
        return _NetvarTypeFor<char*>; // string_t at real
    case ft::FIELD_VECTOR:
        return _extract_type_vec3<netvar_type>(name);
    case ft::FIELD_QUATERNION:
        return _NetvarTypeFor<valve::quaternion>;
    case ft::FIELD_INTEGER:
        return _extract_type_integer<netvar_type>(name);
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
        return _NetvarTypeFor<valve::base_entity*>;
    case ft::FIELD_EHANDLE:
        return _NetvarTypeFor<valve::base_handle>;
    case ft::FIELD_EDICT:
        assert(0 && "Edict field detected"); //  "edict_t*"
        std::unreachable();
    case ft::FIELD_POSITION_VECTOR:
        return _NetvarTypeFor<valve::vector3>;
    case ft::FIELD_TIME:
        return _NetvarTypeFor<float>;
    case ft::FIELD_TICK:
        return _NetvarTypeFor<int32_t>;
    case ft::FIELD_MODELNAME:
    case ft::FIELD_SOUNDNAME:
        return _NetvarTypeFor<char*>; // string_t at real
    case ft::FIELD_INPUT:
        assert(0 && "Inputvar field detected"); //  "CMultiInputVar"
        std::unreachable();
    case ft::FIELD_FUNCTION:
        assert(0 && "Function detected");
        std::unreachable();
    case ft::FIELD_VMATRIX:
    case ft::FIELD_VMATRIX_WORLDSPACE:
        return _NetvarTypeFor<valve::view_matrix>;
    case ft::FIELD_MATRIX3X4_WORLDSPACE:
        return _NetvarTypeFor<valve::matrix3x4>;
    case ft::FIELD_INTERVAL:
        assert(0 && "Interval field detected"); // "interval_t"
        std::unreachable();
    case ft::FIELD_MODELINDEX:
    case ft::FIELD_MATERIALINDEX:
        return _NetvarTypeFor<int32_t>;
    case ft::FIELD_VECTOR2D:
        return _NetvarTypeFor<valve::vector2>;
    default:
        assert(0 && "Unknown datamap field type");
        std::unreachable();
    }
}

static std::optional<custom_netvar_type_simple> _extract_type_by_prefix(std::string_view name, valve::recv_prop* prop)
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
    std::string_view             name,
    valve::data_map_description* field)
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
        [&]<typename T>(T val) -> netvar_type
        {
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