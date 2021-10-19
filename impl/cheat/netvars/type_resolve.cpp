#include "type_resolve.h"

#include "cheat/csgo/CUtlVector.hpp"

#include <nstd/type name.h>
#include <nstd/runtime_assert_fwd.h>

#include <format>

using namespace cheat::detail;
using namespace cheat::csgo;

netvar_type_holder::netvar_type_holder( ) = default;

netvar_type_holder::netvar_type_holder(const std::string_view& sv)
	: std::string(sv.begin( ), sv.end( ))
{
}

netvar_type_holder::netvar_type_holder(std::string&& sv)
	: std::string(std::move(sv))
{
}

std::string cheat::detail::str_to_lower(const std::string_view& str)
{
	std::string ret;
	ret.reserve(str.size( ));
	for (const auto c: str)
		ret += static_cast<char>(std::tolower(c));
	return ret;
}

template <class T>
static constexpr auto _Template_sample = []
{
	auto sample = (nstd::type_name<T>);
	return sample.substr(0, sample.find('<'));
}( );

netvar_type_holder cheat::detail::_As_std_array_type(const std::string_view& type, size_t size)
{
	return std::format("{}<{}, {}>", _Template_sample<std::array<void*, 1>>, type, size);
}

netvar_type_holder cheat::detail::_As_utlvector_type(const std::string_view& type)
{
	return std::format("{}<{}>", _Template_sample<CUtlVector<void*>>, type);
}

netvar_type_holder cheat::detail::_Netvar_vec_type(const std::string_view& name)
{
	const auto is_qangle = [&]
	{
		if (name.starts_with("m_ang"))
			return true;
		auto lstr = str_to_lower(name);
		return lstr.find("angles") != lstr.npos;
	};

	return std::isdigit(name[0]) || !is_qangle( ) ? nstd::type_name<Vector> : nstd::type_name<QAngle>;
}

netvar_type_holder cheat::detail::_Netvar_int_type(std::string_view name)
{
	if (!std::isdigit(name[0]) && name.starts_with("m_"))
	{
		name.remove_prefix(2);
		// ReSharper disable once CppTooWideScopeInitStatement
		const auto is_upper = [&](size_t i)
		{
			return name.size( ) > i && std::isupper(name[i]);
		};
		if (is_upper(1))
		{
			if (name.starts_with('b'))
				return nstd::type_name<bool>;
			if (name.starts_with('c'))
				return nstd::type_name<uint8_t>;
			if (name.starts_with('h'))
				return nstd::type_name<CBaseHandle>;
		}
		else if (is_upper(2))
		{
			if (name.starts_with("un"))
				return nstd::type_name<uint32_t>;
			if (name.starts_with("ch"))
				return nstd::type_name<uint8_t>;
			if (name.starts_with("fl") && str_to_lower(name).find("time") != std::string::npos) //m_flSimulationTime int ???
				return nstd::type_name<float>;
		}
		else if (is_upper(3))
		{
			if (name.starts_with("clr"))
				return nstd::type_name<Color>; //not sure
		}
	}
	return nstd::type_name<int32_t>;
}

//---

netvar_type_holder cheat::detail::_Recv_prop_type(const RecvProp& prop)
{
	switch (prop.m_RecvType)
	{
		case DPT_Int:
			return (_Netvar_int_type(prop.m_pVarName));
		case DPT_Float:
			return (nstd::type_name<float>);
		case DPT_Vector:
			return (_Netvar_vec_type(prop.m_pVarName));
		case DPT_VectorXY:
			return (nstd::type_name<Vector2D>); //3d vector. z unused
		case DPT_String:
			return (nstd::type_name<char*>);
		case DPT_Array:
		{
			const auto& prev_prop = *std::prev(std::addressof(prop));
			runtime_assert(std::string_view(prev_prop.m_pVarName).ends_with("[0]"));
			const auto type = _Recv_prop_type(prev_prop);
			return _As_std_array_type(type, prop.m_nElements);
		}
		case DPT_DataTable:
		{
			runtime_assert("Data table type must be manually resolved!");
			return (nstd::type_name<void*>);
		}
		case DPT_Int64:
			return (nstd::type_name<int64_t>);
		default:
		{
			runtime_assert("Unknown recv prop type");
			return (nstd::type_name<void*>);
		}
	}
}

netvar_type_holder cheat::detail::_Datamap_field_type(const typedescription_t& field)
{
	switch (field.fieldType)
	{
		case FIELD_VOID:
			return nstd::type_name<void*>;
		case FIELD_FLOAT:
			return nstd::type_name<float>;
		case FIELD_STRING:
			return nstd::type_name<char*>; //std::string_t at real
		case FIELD_VECTOR:
			return _Netvar_vec_type(field.fieldName);
		case FIELD_QUATERNION:
		{
			//return "Quaterion";
			runtime_assert("Quaterion field detected");
			return nstd::type_name<void*>;
		}
		case FIELD_INTEGER:
			return _Netvar_int_type(field.fieldName);
		case FIELD_BOOLEAN:
			return nstd::type_name<bool>;
		case FIELD_SHORT:
			return nstd::type_name<int16_t>;
		case FIELD_CHARACTER:
			return nstd::type_name<int8_t>;
		case FIELD_COLOR32:
			return nstd::type_name<Color>;
		case FIELD_EMBEDDED:
		{
			runtime_assert("Embedded field detected");
			return nstd::type_name<void*>;
		}
		case FIELD_CUSTOM:
		{
			runtime_assert("Custom field detected");
			return nstd::type_name<void*>;
		}
		case FIELD_CLASSPTR:
			return nstd::type_name<C_BaseEntity*>;
		case FIELD_EHANDLE:
			return nstd::type_name<CBaseHandle>;
		case FIELD_EDICT:
		{
			//return "edict_t*";
			runtime_assert("Edict field detected");
			return nstd::type_name<void*>;
		}
		case FIELD_POSITION_VECTOR:
			return nstd::type_name<Vector>;
		case FIELD_TIME:
			return nstd::type_name<float>;
		case FIELD_TICK:
			return nstd::type_name<int32_t>;
		case FIELD_MODELNAME:
		case FIELD_SOUNDNAME:
			return nstd::type_name<char*>; //string_t at real
		case FIELD_INPUT:
		{
			//return "CMultiInputVar";
			runtime_assert("Inputvar field detected");
			return nstd::type_name<void*>;
		}
		case FIELD_FUNCTION:
		{
			runtime_assert("Function detected");
			return nstd::type_name<void*>;
		}
		case FIELD_VMATRIX:
		case FIELD_VMATRIX_WORLDSPACE:
			return nstd::type_name<VMatrix>;
		case FIELD_MATRIX3X4_WORLDSPACE:
			return nstd::type_name<matrix3x4_t>;
		case FIELD_INTERVAL:
		{
			//return "interval_t";
			runtime_assert("Interval field detected");
			return nstd::type_name<void*>;
		}
		case FIELD_MODELINDEX:
		case FIELD_MATERIALINDEX:
			return nstd::type_name<int32_t>;
		case FIELD_VECTOR2D:
			return nstd::type_name<Vector2D>;
		default:
		{
			runtime_assert("Unknown datamap field type");
			return nstd::type_name<void*>;
		}
	}
}
