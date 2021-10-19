#pragma once

#include "cheat/csgo/datamap.hpp"
#include "cheat/csgo/Recv.hpp"

#include <string>

namespace cheat::csgo
{
	class matrix3x4_t;
	class VMatrix;
	class Vector2D;
	class Color;
	class QAngle;
	class Vector;
	class CBaseHandle;
}

namespace cheat::detail
{
	struct netvar_type_holder : std::string
	{
		netvar_type_holder();
		netvar_type_holder(const std::string_view& sv);
		netvar_type_holder(std::string&& sv);
		netvar_type_holder(const char*) = delete;
	};

	std::string str_to_lower(const std::string_view& str);

	netvar_type_holder _As_std_array_type(const std::string_view& type, size_t size);
	netvar_type_holder _As_utlvector_type(const std::string_view& type);
	netvar_type_holder _Netvar_vec_type(const std::string_view& name);
	netvar_type_holder _Netvar_int_type(std::string_view name);

	netvar_type_holder _Recv_prop_type(const csgo::RecvProp& prop);
	netvar_type_holder _Datamap_field_type(const csgo::typedescription_t& field);
}
