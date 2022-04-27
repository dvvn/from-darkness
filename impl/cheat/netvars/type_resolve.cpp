module;

#include <nstd/runtime_assert.h>

#include <array>
#include <span>
#include <string>
#include <variant>

module cheat.netvars.core:type_resolve;
import cheat.math.color;
import cheat.math.qangle;
import cheat.math.view_matrix;
import cheat.math.vector2;
import cheat.csgo.tools.UtlVector;
import cheat.csgo.interfaces.BaseHandle;
import nstd.type_name;
import nstd.text.convert;

using nstd::type_name;
using namespace cheat;
using namespace csgo;

std::string netvars::type_std_array(const std::string_view type,const size_t size) noexcept
{
	runtime_assert(size != 0);
	std::string buff;
	constexpr auto arr_name = type_name<std::array>( );
	const auto arr_size = std::to_string(size);
	buff.reserve(arr_name.size( ) + 1 + type.size( ) + 2 + arr_size.size( ) + 1);
	buff += arr_name;
	buff += '<';
	buff += type;
	buff += ", ";
	buff += arr_size;
	buff += '>';
	return buff;
	//return std::format("{}<{}, {}>", type_name<std::array>( ), type, size);
}

std::string netvars::type_utlvector(const std::string_view type) noexcept
{
	std::string buff;
	constexpr auto arr_name = type_name<CUtlVector>( );
	buff.reserve(arr_name.size( ) + 1 + type.size( ) + 1);
	buff += arr_name;
	buff += '<';
	buff += type;
	buff += '>';
	return buff;
	//return std::format("{}<{}>", type_name<CUtlVector>( ), type);
}

//m_xxxX***
static std::string_view _Extract_prefix(const std::string_view type, const size_t prefix_size = 3) noexcept
{
	const auto type_start = 2 + prefix_size;
	if (type.size( ) > type_start && (type[0] == 'm' && type[1] == '_') && std::isupper(type[type_start]))
		return type.substr(2, prefix_size);
	return {};
}

#ifndef __cpp_lib_string_contains
#define contains(_X_) find(_X_) != static_cast<size_t>(-1)
#endif

std::string_view netvars::type_vec3(const std::string_view type) noexcept
{
	const auto vec3_qangle = [=]
	{
		if (std::isdigit(type[0]))
			return false;
		const auto prefix = _Extract_prefix(type);
		if (prefix == "ang")
			return true;
		return nstd::text::to_lower(prefix.empty( ) ? type : type.substr(2)).contains("angles");
	};

	return vec3_qangle( ) ? type_name<math::qangle>( ) : type_name<math::vector3>( );
}

std::string_view netvars::type_integer(std::string_view type) noexcept
{
	if (/*!std::isdigit(type[0]) &&*/ type.starts_with("m_"))
	{
		type.remove_prefix(2);
		const auto is_upper = [&](size_t i)
		{
			return type.size( ) > i && std::isupper(type[i]);
		};
		if (is_upper(1))
		{
			if (type.starts_with('b'))
				return type_name<bool>( );
			if (type.starts_with('c'))
				return type_name<uint8_t>( );
			if (type.starts_with('h'))
				return type_name<CBaseHandle>( );
		}
		else if (is_upper(2))
		{
			if (type.starts_with("un"))
				return type_name<uint32_t>( );
			if (type.starts_with("ch"))
				return type_name<uint8_t>( );
			if (type.starts_with("fl") && nstd::text::to_lower(type.substr(2)).contains("time")) //m_flSimulationTime int ???
				return type_name<float>( );
		}
		else if (is_upper(3))
		{
			if (type.starts_with("clr"))
				return type_name<math::color>( ); //not sure
		}
	}

	return type_name<int32_t>( );
}

//---

string_or_view netvars::type_recv_prop(RecvProp* const prop) noexcept
{
	switch (prop->m_RecvType)
	{
	case DPT_Int:
		return type_integer(prop->m_pVarName);
	case DPT_Float:
		return type_name<float>( );
	case DPT_Vector:
		return type_vec3(prop->m_pVarName);
	case DPT_VectorXY:
		return type_name<math::vector2>( ); //3d vector. z unused
	case DPT_String:
		return type_name<char*>( ); // char[X]
	case DPT_Array:
	{
		const auto prev_prop = std::prev(prop);
		runtime_assert(std::string_view(prev_prop->m_pVarName).ends_with("[0]"));
		const auto type = type_recv_prop(prev_prop);
		return type_std_array(type, prop->m_nElements);
	}
	case DPT_DataTable:
	{
		runtime_assert("Data table type must be manually resolved!");
		return type_name<void*>( );
	}
	case DPT_Int64:
		return type_name<int64_t>( );
	default:
	{
		runtime_assert("Unknown recv prop type");
		return type_name<void*>( );
	}
	}
}

std::string_view netvars::type_datamap_field(typedescription_t* const field)  noexcept
{
	switch (field->fieldType)
	{
	case FIELD_VOID:
		return type_name<void*>( );
	case FIELD_FLOAT:
		return type_name<float>( );
	case FIELD_STRING:
		return type_name<char*>( ); //string_t at real
	case FIELD_VECTOR:
		return type_vec3(field->fieldName);
	case FIELD_QUATERNION:
		return type_name<math::quaternion>( );
	case FIELD_INTEGER:
		return type_integer(field->fieldName);
	case FIELD_BOOLEAN:
		return type_name<bool>( );
	case FIELD_SHORT:
		return type_name<int16_t>( );
	case FIELD_CHARACTER:
		return type_name<int8_t>( );
	case FIELD_COLOR32:
		return type_name<math::color>( );
	case FIELD_EMBEDDED:
	{
		runtime_assert("Embedded field detected");
		return type_name<void*>( );
	}
	case FIELD_CUSTOM:
	{
		runtime_assert("Custom field detected");
		return type_name<void*>( );
	}
	case FIELD_CLASSPTR:
		return type_name<C_BaseEntity*>( );
	case FIELD_EHANDLE:
		return type_name<CBaseHandle>( );
	case FIELD_EDICT:
	{
		//return "edict_t*";
		runtime_assert("Edict field detected");
		return type_name<void*>( );
	}
	case FIELD_POSITION_VECTOR:
		return type_name<math::vector3>( );
	case FIELD_TIME:
		return type_name<float>( );
	case FIELD_TICK:
		return type_name<int32_t>( );
	case FIELD_MODELNAME:
	case FIELD_SOUNDNAME:
		return type_name<char*>( ); //string_t at real
	case FIELD_INPUT:
	{
		//return "CMultiInputVar";
		runtime_assert("Inputvar field detected");
		return type_name<void*>( );
	}
	case FIELD_FUNCTION:
	{
		runtime_assert("Function detected");
		return type_name<void*>( );
	}
	case FIELD_VMATRIX:
	case FIELD_VMATRIX_WORLDSPACE:
		return type_name<math::view_matrix>( );
	case FIELD_MATRIX3X4_WORLDSPACE:
		return type_name<math::matrix3x4>( );
	case FIELD_INTERVAL:
	{
		//return "interval_t";
		runtime_assert("Interval field detected");
		return type_name<void*>( );
	}
	case FIELD_MODELINDEX:
	case FIELD_MATERIALINDEX:
		return type_name<int32_t>( );
	case FIELD_VECTOR2D:
		return type_name<math::vector2>( );
	default:
	{
		runtime_assert("Unknown datamap field type");
		return type_name<void*>( );
	}
	}
}

static std::string_view _Check_int_prefix(const std::string_view type) noexcept
{
	if (_Extract_prefix(type) == "uch")
		return nstd::type_name<math::color>( );
	return {};
}

static std::string_view _Check_float_prefix(const std::string_view type) noexcept
{
	const auto prefix = _Extract_prefix(type);
	if (prefix == "ang")
		return nstd::type_name<math::qangle>( );
	if (prefix == "vec")
		return nstd::type_name<math::vector3>( );
	return {};
}

std::string_view netvars::type_array_prefix(const std::string_view type, csgo::RecvProp* const prop) noexcept
{
	switch (prop->m_RecvType)
	{
	case DPT_Int:
		return _Check_int_prefix(type);
	case DPT_Float:
		return _Check_float_prefix(type);
	default:
		return {};
	}
}

std::string_view netvars::type_array_prefix(const std::string_view type, csgo::typedescription_t* const field) noexcept
{
	switch (field->fieldType)
	{
	case FIELD_INTEGER:
		return _Check_int_prefix(type);
	case FIELD_FLOAT:
		return _Check_float_prefix(type);
	default:
		return {};
	}
}