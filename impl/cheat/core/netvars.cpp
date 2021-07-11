#include "netvars.h"

#include "console.h"
#include "csgo interfaces.h"

#include "cheat/sdk/IBaseClientDll.hpp"
#include "cheat/sdk/entity/C_BaseEntity.h"

#ifdef _DEBUG
#define CHEAT_NETVARS_RESOLVE_TYPE
#endif

using namespace cheat;
using namespace csgo;
using namespace utl;

netvars::~netvars( )
{
}

netvars::netvars( )
{
	this->Wait_for<csgo_interfaces>( );
}

static string _Str_to_lower(const string_view& str)
{
	string ret;
	ret.reserve(str.size( ));
	for (auto& c: str)
		ret += std::tolower(c);
	return ret;
}

template <typename Nstr, typename Tstr>
static bool _Save_netvar(property_tree::ptree& storage, Nstr&& name, int offset, Tstr&& type)
{
#ifdef CHEAT_NETVARS_RESOLVE_TYPE
	static const property_tree::ptree::path_type type_path = _STRINGIZE(type);
#endif

	const auto path = property_tree::ptree::path_type(string(forward<Nstr>(name)));
	if (const auto exists = storage.get_child_optional(path);
		exists.has_value( ))
	{
#ifdef CHEAT_NETVARS_RESOLVE_TYPE
		if (type != _STRINGIZE(void*) && exists->get<string>(type_path) != (type))
			exists->put(type_path, string(forward<Tstr>(type)));
#endif
		return false;
	}

	auto& entry = storage.add_child(path, { });
	entry.put(_STRINGIZE(offset), offset);
#ifdef CHEAT_NETVARS_RESOLVE_TYPE
	entry.put(type_path, string(forward<Tstr>(type)));
#endif
#ifdef _DEBUG
	entry.put("in_use", false);
#endif
	return true;
}

template <typename Nstr>
static pair<property_tree::ptree*, bool> _Add_child_class(property_tree::ptree& storage, Nstr&& name)
{
	string class_name;
	if (name[0] == 'C' && name[1] != '_')
	{
		const auto name1 = string_view(name);
		//internal csgo classes looks like C_***
		//same classes in shared code look like C***
		class_name.reserve(name1.size( ) + 1);
		class_name += "C_";
		class_name.append(name1.begin( ) + 1, name1.end( ));
	}
	else
	{
		class_name = string(forward<Nstr>(name));
		BOOST_ASSERT(!class_name.starts_with("DT_"));
	}

	property_tree::ptree* tree;

	const auto child = storage.get_child_optional(class_name);
	if (!child)
		tree = addressof(storage.add_child(move(class_name), { }));
	else
		tree = child.get_ptr( );

	return {tree, !child};
}

static string _Array_type_string(const string_view& type, size_t size)
{
	return fmt::format("utl::array<{}, {}>", type, size);
}

static string _Netvar_vec_type(const string_view& name)
{
	// ReSharper disable once CppTooWideScopeInitStatement
	const auto is_qangle = [&]
	{
		if (name.starts_with("m_ang"))
			return true;
		auto lstr = _Str_to_lower(name);
		return lstr.find("angles") != lstr.npos;
	};

	if (std::isdigit(name[0]) || !is_qangle( ))
		return _STRINGIZE(utl::Vector);
	else
		return _STRINGIZE(utl::Qangle);
}

static string _Netvar_int_type(string_view name)
{
	if (!std::isdigit(name[0]) && name.starts_with("m_"))
	{
		name.remove_prefix(2);
		const auto is_upper = [&](size_t i)
		{
			return name.size( ) > i && std::isupper(name[i]);
		};

		if (is_upper(1))
		{
			if (name.starts_with('b'))
				return _STRINGIZE(bool);
			if (name.starts_with('c'))
				return _STRINGIZE(uint8_t);
		}
		else if (is_upper(2))
		{
			if (name.starts_with("un"))
				return _STRINGIZE(uint32_t);
			if (name.starts_with("ch"))
				return _STRINGIZE(uint8_t);
		}
		if (is_upper(3))
		{
			if (name.starts_with("clr"))
				return _STRINGIZE(utl::Color); //not sure
		}
	}
	return _STRINGIZE(int32_t);
}

static string _Recv_prop_type(const RecvProp& prop)
{
	switch (prop.m_RecvType)
	{
		case DPT_Int:
			return _Netvar_int_type(prop.m_pVarName);
		case DPT_Float:
			return _STRINGIZE(float);
		case DPT_Vector:
			return _Netvar_vec_type(prop.m_pVarName);
		case DPT_VectorXY:
			return _STRINGIZE(utl::Vector2D); //3d vector. z unused
		case DPT_String:
			return _STRINGIZE(char*);
		case DPT_Array:
		{
			const auto& prev_prop = *std::prev(addressof(prop));
			BOOST_ASSERT(string_view(prev_prop.m_pVarName).ends_with("[0]"));
			const auto type = _Recv_prop_type(prev_prop);
			return _Array_type_string(type, prop.m_nElements);
		}
		case DPT_DataTable:
		{
			BOOST_ASSERT("Data table type must be manually resolved!");
			return _STRINGIZE(void*);
		}
		case DPT_Int64:
			return _STRINGIZE(int64_t);
		default:
		{
			BOOST_ASSERT("Unknown recv prop type");
			return _STRINGIZE(void*);
		}
	}
}

static string _Datamap_field_type(const typedescription_t& field)
{
	switch (field.fieldType)
	{
		case FIELD_VOID:
			return _STRINGIZE(void*);
		case FIELD_FLOAT:
			return _STRINGIZE(float);
		case FIELD_STRING:
			return _STRINGIZE(char*); //string_t at real
		case FIELD_VECTOR:
			return _Netvar_vec_type(field.fieldName);
		case FIELD_QUATERNION:
			return _STRINGIZE(utl::Quaterion);
		case FIELD_INTEGER:
			return _Netvar_int_type(field.fieldName);
		case FIELD_BOOLEAN:
			return _STRINGIZE(bool);
		case FIELD_SHORT:
			return _STRINGIZE(int16_t);
		case FIELD_CHARACTER:
			return _STRINGIZE(int8_t);
		case FIELD_COLOR32:
			return _STRINGIZE(utl::Color);
		case FIELD_EMBEDDED:
		{
			BOOST_ASSERT("Embedded field detected");
			return _STRINGIZE(void*);
		}
		case FIELD_CUSTOM:
		{
			BOOST_ASSERT("Custom field detected");
			return _STRINGIZE(void*);
		}
		case FIELD_CLASSPTR:
			return _STRINGIZE(C_BaseEntity*);
		case FIELD_EHANDLE:
		{
			BOOST_ASSERT("Ent handle detected");
			return _STRINGIZE(void*); //todo
		}
		case FIELD_EDICT:
			return _STRINGIZE(edict_t*);
		case FIELD_POSITION_VECTOR:
			return _STRINGIZE(utl::Vector);
		case FIELD_TIME:
			return _STRINGIZE(float);
		case FIELD_TICK:
			return _STRINGIZE(int32_t);
		case FIELD_MODELNAME:
		case FIELD_SOUNDNAME:
			return _STRINGIZE(char*); //string_t at real
		case FIELD_INPUT:
			return _STRINGIZE(CMultiInputVar);
		case FIELD_FUNCTION:
		{
			BOOST_ASSERT("Function detected");
			return _STRINGIZE(void*);
		}
		case FIELD_VMATRIX:
		case FIELD_VMATRIX_WORLDSPACE:
			return _STRINGIZE(utl::VMatrix);
		case FIELD_MATRIX3X4_WORLDSPACE:
			return _STRINGIZE(utl::matrix3x4_t);
		case FIELD_INTERVAL:
			return _STRINGIZE(interval_t);
		case FIELD_MODELINDEX:
		case FIELD_MATERIALINDEX:
			return _STRINGIZE(int32_t);
		case FIELD_VECTOR2D:
			return _STRINGIZE(utl::Vector2D);
		default:
		{
			BOOST_ASSERT("Unknown datamap field type");
			return _STRINGIZE(void*);
		}
	}
}

static void _Store_recv_props(property_tree::ptree& root_tree, property_tree::ptree& tree, RecvTable* recv_table, int offset)
{
	const static auto prop_is_length_proxy = [](const RecvProp& prop)
	{
		if (prop.m_ArrayLengthProxy)
			return true;
		const auto lstr = _Str_to_lower(prop.m_pVarName);
		return lstr.find("length") != lstr.npos && lstr.find("proxy") != lstr.npos;
	};

	const static auto prop_is_base_class = [](const RecvProp& prop)
	{
		return prop.m_pVarName == string_view("baseclass");
	};

	const static auto table_is_array = [](const RecvTable& table)
	{
		return !table.props.empty( ) && std::isdigit(table.props.back( ).m_pVarName[0]);
	};

	const static auto table_is_data_table = [](const RecvTable& table)
	{
		//DT_XXXXXX
		auto n = table.m_pNetTableName;
		return n[2] == '_' && n[0] == 'D' && n[1] == 'T';
	};

	// ReSharper disable once CppTooWideScopeInitStatement
	const auto props = [&]
	{
		const auto& raw_props = recv_table->props;

		auto front = raw_props.begin( );
		auto back = std::prev(raw_props.end( ));

		if (prop_is_base_class(*front))
			++front;
		if (prop_is_length_proxy(*back))
			--back;

		return span(front, std::next(back));
	}( );

	for (auto itr = props.begin( ); itr != props.end( ); ++itr)
	{
		const auto& prop = *itr;
		const auto  prop_name = string_view(prop.m_pVarName);

		if (static const auto path_default_separator = property_tree::ptree::path_type( ).separator( );
			prop_name.find(path_default_separator) != prop_name.npos)
		{
			continue;
		}
		else if (path_default_separator != '.' && prop_name.rfind('.') != prop_name.npos)
		{
			continue;
		}

		const auto real_prop_offset = offset + prop.m_Offset;

		if (prop_name.rfind(']') != prop_name.npos)
		{
			if (prop_name.ends_with("[0]"))
			{
				const auto real_prop_name = string_view(prop_name.begin( ), prop_name.end( ) - 3);
				BOOST_ASSERT(!real_prop_name.ends_with(']'));
				auto array_size = optional<size_t>(1);

				// ReSharper disable once CppUseStructuredBinding
				for (const auto& p: span(std::next(itr), props.end( )))
				{
					if (const auto name = string_view(p.m_pVarName); name.starts_with(real_prop_name))
					{
						if (p.m_RecvType == prop.m_RecvType && name.size( ) != real_prop_name.size( ))
						{
							++*array_size;
						}
						else
						{
							array_size.reset( );
							break;
						}
					}
				}

				if (array_size.has_value( ))
				{
#ifdef CHEAT_NETVARS_RESOLVE_TYPE
					string netvar_type;
					if (*array_size == 3 && (std::isupper(real_prop_name[5]) && real_prop_name.starts_with("m_")))
					{
						auto prefix = real_prop_name.substr(2, 3);
						if (prop.m_RecvType == DPT_Float)
						{
							if ((prefix == "ang" || prefix == "vec"))
								netvar_type = _Netvar_vec_type(real_prop_name);
						}
						else if (prop.m_RecvType == DPT_Int)
						{
							if (prefix == "uch")
								netvar_type = _STRINGIZE(utl::Color); //todo: move to func
						}
					}

					if (netvar_type.empty( ))
					{
						auto type = _Recv_prop_type(prop);
						netvar_type = _Array_type_string(type, *array_size);
					}
#else
					auto netvar_type = nullptr;
#endif
					_Save_netvar(tree, real_prop_name, real_prop_offset, move(netvar_type));

					itr += *array_size - 1;
				}
			}
			continue;
		}
#if 0
		optional<size_t> array_size;
		if (std::isdigit(prop_name[0]))
		{
			BOOST_ASSERT(prop_name[0] == '0');

			const auto part = props.subspan(i + 1);
			const auto array_end_itr = ranges::find_if_not(part, [](const RecvProp& rp) { return std::isdigit(rp.m_pVarName[0]); });
			const auto array_end_num = std::distance(part.begin( ), array_end_itr);

			array_size = array_end_num - i + 1;
			i += array_end_num;

			static const string props_array = "m_PropsArray";
			prop_name = props_array;
		}

		if (prop.m_ArrayLengthProxy != nullptr)
		{
			continue;
		}
#else
		BOOST_ASSERT(!prop_is_length_proxy(prop));
		BOOST_ASSERT(!std::isdigit(prop_name[0]));
#endif

		if (prop.m_RecvType != DPT_DataTable)
		{
#ifdef CHEAT_NETVARS_RESOLVE_TYPE
			auto netvar_type = _Recv_prop_type(prop);
#else
			const auto netvar_type = nullptr;
#endif
			_Save_netvar(tree, prop_name, real_prop_offset, move(netvar_type));
		}
		else
		{
			const auto  child_table = prop.m_pDataTable;
			const auto& child_props = child_table->props;
			if (!child_table || child_props.empty( ))
				continue;

			if (table_is_data_table(*child_table))
			{
				_Store_recv_props(root_tree, tree, child_table, real_prop_offset);
			}
			else if (table_is_array(*child_table))
			{
				auto array_begin = child_props.begin( );
				if (prop_is_length_proxy(*array_begin))
				{
					++array_begin;
					BOOST_ASSERT(array_begin->m_pVarName[0] == '0');
				}
				BOOST_ASSERT(array_begin != child_props.end());

#ifdef _DEBUG
				// ReSharper disable once CppUseStructuredBinding
				for (const auto& rp: span(array_begin + 1, child_props.end( )))
				{
					BOOST_ASSERT(std::isdigit(rp.m_pVarName[0]));
					BOOST_ASSERT(rp.m_RecvType == array_begin->m_RecvType);
				}
#endif

#ifdef CHEAT_NETVARS_RESOLVE_TYPE
				const auto array_size = std::distance(array_begin, child_props.end( ));
				string     netvar_type;
#endif
				if (array_begin->m_RecvType != DPT_DataTable)
				{
#ifdef CHEAT_NETVARS_RESOLVE_TYPE
					netvar_type = _Recv_prop_type(*array_begin);
#endif
				}
				else
				{
					const auto child_table_name = string_view(child_table->m_pNetTableName);
					string     child_table_unique_name;
					if (prop_name != child_table_name)
					{
						child_table_unique_name = child_table_name;
					}
					else
					{
						constexpr auto unique_str = string_view("_t");
						child_table_unique_name.reserve(child_table_name.size( ) + unique_str.size( ));
						child_table_unique_name.append(child_table_name);
						child_table_unique_name.append(unique_str);
					}
					auto [new_tree, added] = _Add_child_class(root_tree, child_table_unique_name);
					if (!added)
						continue;
#ifdef CHEAT_NETVARS_RESOLVE_TYPE
					netvar_type = move(child_table_unique_name);
#endif
					_Store_recv_props(root_tree, *new_tree, array_begin->m_pDataTable, /*real_prop_offset*/0);
				}

#ifdef CHEAT_NETVARS_RESOLVE_TYPE
				auto netvar_type_array = _Array_type_string(netvar_type, array_size);
#else
				auto netvar_type_array = nullptr;
#endif
				_Save_netvar(tree, prop_name, real_prop_offset, move(netvar_type_array));
			}
			else
			{
				BOOST_ASSERT("Unknown netvar type");
			}
		}
	}
}

[[maybe_unused]]
static void _Iterate_client_class(property_tree::ptree& root_tree, ClientClass* root_class)
{
	for (auto client_class = root_class; client_class != nullptr; client_class = client_class->pNext)
	{
		const auto recv_table = client_class->pRecvTable;
		if (!recv_table || recv_table->props.empty( ))
			continue;

		auto [new_tree, added] = _Add_child_class(root_tree, client_class->pNetworkName);
		BOOST_ASSERT(added == true);

		_Store_recv_props(root_tree, *new_tree, recv_table, 0);

		if (new_tree->empty( ))
			root_tree.pop_back( );
	}
}

static void _Store_datamap_props(property_tree::ptree& tree, datamap_t* map)
{
	// ReSharper disable once CppUseStructuredBinding
	for (auto& desc: map->data)
	{
		BOOST_ASSERT(desc.fieldName != nullptr);

		if (desc.fieldType != FIELD_EMBEDDED)
		{
			const auto offset = desc.fieldOffset[TD_OFFSET_NORMAL];

#ifdef CHEAT_NETVARS_RESOLVE_TYPE
			auto field_type = _Datamap_field_type(desc);
#else
			const auto field_type=nullptr;
#endif
			_Save_netvar(tree, desc.fieldName, offset, move(field_type));
		}
		else
		{
			if (desc.TypeDescription != nullptr)
			{
				BOOST_ASSERT("Embedded datamap detected");
			}
		}
	}
}

[[maybe_unused]]
static void _Iterate_datamap(property_tree::ptree& root_tree, datamap_t* root_map)
{
	for (auto map = root_map; map != nullptr; map = map->baseMap)
	{
		if (map->data.empty( ))
			continue;

		auto [tree, added] = _Add_child_class(root_tree, map->dataClassName);

		_Store_datamap_props(*tree, map);

		if (added && tree->empty( ))
			root_tree.pop_back( );
	}
}

void netvars::Load( )
{
#ifdef CHEAT_GUI_TEST

#else

#ifdef _DEBUG
	data__.clear( );
#endif

	_Iterate_client_class(data__, csgo_interfaces::get( ).client->GetAllClasses( ));

	const auto client_dll = mem::all_modules::get( ).find("client.dll");
	auto&      vtables = client_dll->vtables( );

	[[maybe_unused]] filesystem::path dumps_path;
#if defined(_DEBUG) /*|| 1*/ //dump first time in release mode, debug mode extremely slow!
	dumps_path = filesystem::path(CHEAT_DUMPS_FOLDER) / client_dll->name_wide( ) / to_wstring(client_dll->check_sum( )) / L"vtables.json";
#endif

	[[maybe_unused]] const auto load_result = vtables.load(dumps_path);
	BOOST_ASSERT(load_result == mem::data_cache_result::success);

	const auto baseent = vtables.get_cache( ).at(_STRINGIZE(C_BaseEntity)).addr.raw<C_BaseEntity>( );

	_Iterate_datamap(data__, baseent->GetDataDescMap( ));
	_Iterate_datamap(data__, baseent->GetPredictionDescMap( ));

#endif
}

string netvars::Get_loaded_message( ) const
{
#ifdef CHEAT_GUI_TEST
	return { };
#else
	return service_base::Get_loaded_message( );
#endif
}

void netvars::Post_load( )
{
	//todo: run dumper here
}

int netvars::at(const string_view& path) const
{
	auto& child = data__.get_child(string(path));

#ifdef _DEBUG
	auto& child1 = const_cast<property_tree::ptree&>(child);
	BOOST_ASSERT_MSG(child1.get<bool>("in_use") == false, "Netvar already used!");
	child1.put("in_use", true);
#endif

	return child.get<int>("offset");
}
