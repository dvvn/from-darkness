module;

//#include "cheat/service/includes.h"

#include <nstd/type name.h>
#include <nstd/runtime_assert.h>
#include <nstd/overload.h>
#include <nstd/format.h>
#include <nstd/ranges.h>

#include <array>
#include <span>
#include <string>
#include <variant>

module cheat.netvars:type_resolve;
import nstd.text.actions;
import nstd.container.wrapper;

template<typename Rng>
static auto _To_lower(const Rng& rng)
{
	std::basic_string<std::ranges::range_value_t<Rng>> out;
	auto tmp = rng | std::views::transform(nstd::text::to_lower);
	nstd::container::append(out, tmp);
	return out;
}

using nstd::type_name;
using nstd::overload;
using namespace cheat;
using namespace csgo;
using netvars_impl::string_or_view_holder;

string_or_view_holder::string_or_view_holder( )
{
	str_.emplace<std::string_view>("");
}

string_or_view_holder::string_or_view_holder(const std::string_view& sv)
{
	str_.emplace<std::string_view>(sv);
}

string_or_view_holder::string_or_view_holder(std::string&& str)
{
	str_.emplace<std::string>(std::move(str));
}

std::string& string_or_view_holder::str( )&
{
	return std::visit(overload([](std::string& str)-> std::string& { return str; }
	, [](std::string_view&)-> std::string& { throw std::logic_error("Incorrect string type!"); }), str_);
}

std::string string_or_view_holder::str( )&&
{
	return std::visit(overload([](std::string&& str)-> std::string { return std::move(str); }
	, [](std::string_view&& sv)-> std::string { return {sv.begin( ), sv.end( )}; }), std::move(str_));
}

std::string_view string_or_view_holder::view( )const&
{
	return std::visit(overload([]<typename T>(T && obj)-> std::string_view { return {obj.data( ), obj.size( )}; }), str_);
}

string_or_view_holder netvars_impl::type_std_array(const std::string_view& type, size_t size)
{
	runtime_assert(size != 0);
	return std::format("{}<{}, {}>", type_name<std::array>( ), type, size);
}

string_or_view_holder netvars_impl::type_utlvector(const std::string_view& type)
{
	return std::format("{}<{}>", type_name<CUtlVector>( ), type);
}

string_or_view_holder netvars_impl::type_vec3(const std::string_view& name)
{
	const auto is_qangle = [&]
	{
		if (name.starts_with("m_ang"))
			return true;
		auto lstr = _To_lower(name);
		return lstr.find("angles") != lstr.npos;
	};
	return std::isdigit(name[0]) || !is_qangle( ) ? type_name<Vector>( ) : type_name<QAngle>( );
}

string_or_view_holder netvars_impl::type_integer(std::string_view name)
{
	if (/*!std::isdigit(name[0]) &&*/ name.starts_with("m_"))
	{
		name.remove_prefix(2);
		const auto is_upper = [&](size_t i)
		{
			return name.size( ) > i && std::isupper(name[i]);
		};
		if (is_upper(1))
		{
			if (name.starts_with('b'))
				return type_name<bool>( );
			if (name.starts_with('c'))
				return type_name<uint8_t>( );
			if (name.starts_with('h'))
				return type_name<CBaseHandle>( );
		}
		else if (is_upper(2))
		{
			if (name.starts_with("un"))
				return type_name<uint32_t>( );
			if (name.starts_with("ch"))
				return type_name<uint8_t>( );
			if (name.starts_with("fl") && _To_lower(name).find("time") != std::string::npos) //m_flSimulationTime int ???
				return type_name<float>( );
		}
		else if (is_upper(3))
		{
			if (name.starts_with("clr"))
				return type_name<Color>( ); //not sure
		}
	}

	return type_name<int32_t>( );
}

//---

string_or_view_holder netvars_impl::type_recv_prop(const RecvProp& prop)
{
	switch (prop.m_RecvType)
	{
	case DPT_Int:
		return type_integer(prop.m_pVarName);
	case DPT_Float:
		return type_name<float>( );
	case DPT_Vector:
		return type_vec3(prop.m_pVarName);
	case DPT_VectorXY:
		return type_name<Vector2D>( ); //3d vector. z unused
	case DPT_String:
		return type_name<char*>( ); // char[X]
	case DPT_Array:
	{
		const auto& prev_prop = *std::prev(std::addressof(prop));
		runtime_assert(std::string_view(prev_prop.m_pVarName).ends_with("[0]"));
		const auto type = type_recv_prop(prev_prop);
		return type_std_array(type.view( ), prop.m_nElements);
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

string_or_view_holder netvars_impl::type_datamap_field(const typedescription_t& field)
{
	switch (field.fieldType)
	{
	case FIELD_VOID:
		return type_name<void*>( );
	case FIELD_FLOAT:
		return type_name<float>( );
	case FIELD_STRING:
		return type_name<char*>( ); //std::string_t at real
	case FIELD_VECTOR:
		return type_vec3(field.fieldName);
	case FIELD_QUATERNION:
	{
		//return "Quaternion";
		runtime_assert("Quaternion field detected");
		return type_name<void*>( );
	}
	case FIELD_INTEGER:
		return type_integer(field.fieldName);
	case FIELD_BOOLEAN:
		return type_name<bool>( );
	case FIELD_SHORT:
		return type_name<int16_t>( );
	case FIELD_CHARACTER:
		return type_name<int8_t>( );
	case FIELD_COLOR32:
		return type_name<Color>( );
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
		return type_name<Vector>( );
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
		return type_name<VMatrix>( );
	case FIELD_MATRIX3X4_WORLDSPACE:
		return type_name<matrix3x4_t>( );
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
		return type_name<Vector2D>( );
	default:
	{
		runtime_assert("Unknown datamap field type");
		return type_name<void*>( );
	}
	}
}
