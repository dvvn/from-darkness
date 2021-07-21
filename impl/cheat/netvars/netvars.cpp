#include "netvars.h"

#include "cheat/core/console.h"
#include "cheat/core/csgo interfaces.h"

#include "cheat/sdk/IBaseClientDll.hpp"
#include "cheat/sdk/IVEngineClient.hpp"
#include "cheat/sdk/entity/C_BaseEntity.h"
#include "cheat/sdk/entity/C_CSPlayer.h"

#ifdef _DEBUG
#define CHEAT_NETVARS_RESOLVE_TYPE
#endif
#if 0
#define CHEAT_NETVARS_DUMP_STATIC_OFFSET
#endif
#if defined(CHEAT_GUI_TEST) || (defined(CHEAT_NETVARS_DUMP_STATIC_OFFSET) && !defined(_DEBUG))
#define CHEAT_NETVARS_DUMPER_DISABLED
#endif

using namespace cheat;
using namespace csgo;
using namespace utl;

// ReSharper disable once CppInconsistentNaming
static const auto PATH_DEFAULT_SEPARATOR = property_tree::ptree::path_type( ).separator( );

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

struct variable_info
{
	template <typename T>
	struct info: string_view
	{
		using type = T;

		// ReSharper disable once CppRedundantInlineSpecifier
		_CONSTEXPR20_CONTAINER operator string( ) const
		{
			return string(this->_Unchecked_begin( ), this->_Unchecked_end( ));
		}

		operator property_tree::ptree::path_type( ) const
		{
			return {static_cast<string>(*this)};
		}

		T ptree_get(const property_tree::ptree& tree) const
		{
			return tree.get<T>(*this);
		}

		property_tree::ptree& ptree_put(property_tree::ptree& tree, const T& value) const
		{
			return tree.put(*this, value);
		}

		property_tree::ptree& ptree_put(property_tree::ptree& tree, T&& value) const
		{
			return tree.put(*this, move(value));
		}
	};
};

struct netvar_info: variable_info
{
	static constexpr auto offset = info<size_t>("offset");
#ifdef CHEAT_NETVARS_RESOLVE_TYPE
	static constexpr auto type = info<string>("type");
#endif
	//#ifdef _DEBUG
	//	static constexpr auto in_use = info<bool>("in_use");
	//#endif
};

template <typename Nstr>
static bool _Save_netvar(property_tree::ptree& storage, Nstr&& name, int offset, string&& type)
{
	const auto path = property_tree::ptree::path_type(string(forward<Nstr>(name)));
	if (const auto exists = storage.get_child_optional(path);
		exists.has_value( ))
	{
#ifdef CHEAT_NETVARS_RESOLVE_TYPE
		if (type != "void*" && netvar_info::type.ptree_get(*exists) != type)
			netvar_info::type.ptree_put(*exists, move(type));
#endif
		return false;
	}

	auto& entry = storage.add_child(path, { });
	netvar_info::offset.ptree_put(entry, offset);
#ifdef CHEAT_NETVARS_RESOLVE_TYPE
	netvar_info::type.ptree_put(entry, move(type));
#endif
	//#ifdef _DEBUG
	//	netvar_info::in_use.ptree_put(entry, false);
	//#endif
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
	return format("utl::array<{}, {}>", type, size);
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

	return string("utl::") + (std::isdigit(name[0]) || !is_qangle( ) ? "Vector" : "QAngle");
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
				return "bool";
			if (name.starts_with('c'))
				return "uint8_t";
		}
		else if (is_upper(2))
		{
			if (name.starts_with("un"))
				return "uint32_t";
			if (name.starts_with("ch"))
				return "uint8_t";
			if (name.starts_with("fl") && _Str_to_lower(name).find("time") != string::npos) //m_flSimulationTime int ???
				return "float";
		}
		if (is_upper(3))
		{
			if (name.starts_with("clr"))
				return "utl::Color"; //not sure
		}
	}
	return "int32_t";
}

static string _Recv_prop_type(const RecvProp& prop)
{
	switch (prop.m_RecvType)
	{
		case DPT_Int:
			return _Netvar_int_type(prop.m_pVarName);
		case DPT_Float:
			return "float";
		case DPT_Vector:
			return _Netvar_vec_type(prop.m_pVarName);
		case DPT_VectorXY:
			return "utl::Vector2D"; //3d vector. z unused
		case DPT_String:
			return "char*";
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
			return "void*";
		}
		case DPT_Int64:
			return "int64_t";
		default:
		{
			BOOST_ASSERT("Unknown recv prop type");
			return "void*";
		}
	}
}

static string _Datamap_field_type(const typedescription_t& field)
{
	switch (field.fieldType)
	{
		case FIELD_VOID:
			return "void*";
		case FIELD_FLOAT:
			return "float";
		case FIELD_STRING:
			return "char*"; //string_t at real
		case FIELD_VECTOR:
			return _Netvar_vec_type(field.fieldName);
		case FIELD_QUATERNION:
		{
			//return "utl::Quaterion";
			BOOST_ASSERT("Quaterion field detected");
			return "void*";
		}
		case FIELD_INTEGER:
			return _Netvar_int_type(field.fieldName);
		case FIELD_BOOLEAN:
			return "bool";
		case FIELD_SHORT:
			return "int16_t";
		case FIELD_CHARACTER:
			return "int8_t";
		case FIELD_COLOR32:
			return "utl::Color";
		case FIELD_EMBEDDED:
		{
			BOOST_ASSERT("Embedded field detected");
			return "void*";
		}
		case FIELD_CUSTOM:
		{
			BOOST_ASSERT("Custom field detected");
			return "void*";
		}
		case FIELD_CLASSPTR:
			return "C_BaseEntity*";
		case FIELD_EHANDLE:
		{
			BOOST_ASSERT("Ent handle detected");
			return "void*"; //todo
		}
		case FIELD_EDICT:
		{
			//return "edict_t*";
			BOOST_ASSERT("Edict field detected");
			return "void*";
		}
		case FIELD_POSITION_VECTOR:
			return "utl::Vector";
		case FIELD_TIME:
			return "float";
		case FIELD_TICK:
			return "int32_t";
		case FIELD_MODELNAME:
		case FIELD_SOUNDNAME:
			return "char*"; //string_t at real
		case FIELD_INPUT:
		{
			//return "CMultiInputVar";
			BOOST_ASSERT("Inputvar field detected");
			return "void*";
		}
		case FIELD_FUNCTION:
		{
			BOOST_ASSERT("Function detected");
			return "void*";
		}
		case FIELD_VMATRIX:
		case FIELD_VMATRIX_WORLDSPACE:
			return "VMatrix";
		case FIELD_MATRIX3X4_WORLDSPACE:
			return "matrix3x4_t";
		case FIELD_INTERVAL:
		{
			//return "interval_t";
			BOOST_ASSERT("Interval field detected");
			return "void*";
		}
		case FIELD_MODELINDEX:
		case FIELD_MATERIALINDEX:
			return "int32_t";
		case FIELD_VECTOR2D:
			return "utl::Vector2D";
		default:
		{
			BOOST_ASSERT("Unknown datamap field type");
			return "void*";
		}
	}
}

static void _Store_recv_props(property_tree::ptree& root_tree, property_tree::ptree& tree, RecvTable* recv_table, int offset)
{
	static constexpr auto prop_is_length_proxy = [](const RecvProp& prop)
	{
		if (prop.m_ArrayLengthProxy)
			return true;
		const auto lstr = _Str_to_lower(prop.m_pVarName);
		return lstr.find("length") != lstr.npos && lstr.find("proxy") != lstr.npos;
	};

	static constexpr auto prop_is_base_class = [](const RecvProp& prop)
	{
		return prop.m_pVarName == string_view("baseclass");
	};

	static constexpr auto table_is_array = [](const RecvTable& table)
	{
		return !table.props.empty( ) && std::isdigit(table.props.back( ).m_pVarName[0]);
	};

	static constexpr auto table_is_data_table = [](const RecvTable& table)
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
		BOOST_ASSERT(prop.m_pVarName != nullptr);
		const auto prop_name = string_view(prop.m_pVarName);

		if (prop_name.find(PATH_DEFAULT_SEPARATOR) != prop_name.npos)
		{
			continue;
		}
		if (PATH_DEFAULT_SEPARATOR != '.' && prop_name.rfind('.') != prop_name.npos)
		{
			continue;
		}

		const auto real_prop_offset = offset + prop.m_Offset;

		if (prop_name.rfind(']') != prop_name.npos)
		{
			if (prop_name.ends_with("[0]"))
			{
				const auto real_prop_name = string_view(prop_name.begin( ), std::prev(prop_name.end( ), 3));
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
							if (prefix == "ang")
								netvar_type = "utl::QAngle";
							else if (prefix == "vec")
								netvar_type = "utl::Vector";
						}
						else if (prop.m_RecvType == DPT_Int)
						{
							if (prefix == "uch")
								netvar_type = "utl::Color";
						}
					}

					if (netvar_type.empty( ))
					{
						auto type = _Recv_prop_type(prop);
						netvar_type = _Array_type_string(type, *array_size);
					}
#else
					const auto netvar_type = nullptr;
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
			string netvar_type;
#endif
			_Save_netvar(tree, prop_name, real_prop_offset, move(netvar_type));
		}
		else
		{
			const auto child_table = prop.m_pDataTable;
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
				string netvar_type;
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
					string child_table_unique_name;
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
				string netvar_type_array;
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
		if (desc.fieldType == FIELD_EMBEDDED)
		{
			if (desc.TypeDescription != nullptr)
			{
				BOOST_ASSERT("Embedded datamap detected");
			}
		}
		else if (desc.fieldName != nullptr)
		{
			const auto offset = desc.fieldOffset[TD_OFFSET_NORMAL];

#ifdef CHEAT_NETVARS_RESOLVE_TYPE
			auto field_type = _Datamap_field_type(desc);
#else
			string field_type;
#endif
			_Save_netvar(tree, desc.fieldName, offset, move(field_type));
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

		auto&& [tree, added] = _Add_child_class(root_tree, map->dataClassName);

		_Store_datamap_props(*tree, map);

		if (added && tree->empty( ))
			root_tree.pop_back( );
	}
}

void netvars::Load( )
{
#if !defined(CHEAT_NETVARS_DUMPER_DISABLED)

#ifdef _DEBUG
	data__.clear( );
#endif

	const auto& interfaces = csgo_interfaces::get( );

	_Iterate_client_class(data__, interfaces.client->GetAllClasses( ));

	const auto baseent = [&]
	{
#ifndef CHEAT_NETVARS_UPDATING
		if (interfaces.local_player != nullptr)
		{
			if (C_BaseEntity* localp = interfaces.local_player; localp != nullptr)
				return localp;
		}
#endif
		const auto client_dll = all_modules::get( ).find("client.dll");
		auto& vtables = client_dll->vtables( );

		[[maybe_unused]] filesystem::path dumps_path;
#if defined(_DEBUG) || defined(CHEAT_NETVARS_UPDATING)
		dumps_path = filesystem::path(CHEAT_DUMPS_DIR) / client_dll->name_wide( ) / to_wstring(client_dll->check_sum( )) / L"vtables.json";
#endif

		[[maybe_unused]] const auto load_result = vtables.load(dumps_path);
		BOOST_ASSERT(load_result == success);

		return vtables.get_cache( ).at("C_BaseEntity").addr.raw<C_BaseEntity>( );
	}( );

	_Iterate_datamap(data__, baseent->GetDataDescMap( ));
	_Iterate_datamap(data__, baseent->GetPredictionDescMap( ));

#endif
}

string netvars::Get_loaded_message( ) const
{
#ifndef CHEAT_NETVARS_DUMPER_DISABLED
	return service_base::Get_loaded_message( );
#else
	return Get_loaded_message_disabled( );
#endif
}

void netvars::Post_load( )
{
#if defined(CHEAT_NETVARS_RESOLVE_TYPE) && !defined(CHEAT_NETVARS_DUMPER_DISABLED)

	Dump_netvars_( );
	Generate_classes_( );

#endif
}

static string _Get_checksum(const filesystem::path& p, bool first_time)
{
	auto checksum = string( );
	using itr_t = std::istreambuf_iterator<char>;
	if (auto ifs = std::ifstream(p); !first_time && exists(p) && !ifs.fail( ))
		checksum = {itr_t(ifs), itr_t( )};

	return checksum;
}

static string _Get_checksum(const string_view& s)
{
	return to_string(invoke(std::hash<string_view>( ), s));
}

static string _Get_checksum(const std::ostringstream& ss)
{
	return _Get_checksum(ss.str( ));
}

void netvars::Dump_netvars_( )
{
	const auto netvars_dump_dir = filesystem::path(CHEAT_DUMPS_DIR) / L"netvars";
	const auto first_time = create_directories(netvars_dump_dir);

	constexpr auto get_file_name = []
	{
		string version = csgo_interfaces::get( ).engine->GetProductVersionString( );
		ranges::replace(version, '.', '_');
		version.append(".json");
		return version;
	};

	if (const auto netvars_dump_file = netvars_dump_dir / get_file_name( );
		!exists(netvars_dump_file))
	{
		std::ofstream file(netvars_dump_file);
		write_json(file, data__);
	}
	else
	{
		const auto test_to_write = [&]
		{
			std::ostringstream ss;
			write_json(ss, data__);
			return ss.str( );
		}( );

		if (_Get_checksum(test_to_write) == _Get_checksum(netvars_dump_file, first_time))
			return;

		std::ofstream file(netvars_dump_file);
		file << (test_to_write);
	}

#ifdef CHEAT_HAVE_CONSOLE
	console::get_ptr( )->write_line("Netvars dump done");
#endif
}

void netvars::Generate_classes_( )
{
	const auto dir = filesystem::path(CHEAT_IMPL_DIR) / L"sdk" / L"generated";
#if 0
	remove_all(dir);
	create_directories(dir);
#else
	const auto first_time = create_directories(dir);

	const auto checksum_file = dir / L"__checksum.txt";
	const auto last_checksum = _Get_checksum(checksum_file, first_time);

	const auto current_checksum = [&]
	{
		std::ostringstream ss;
		write_json(ss, data__, false);
		return _Get_checksum(ss);
	}( );

	if (last_checksum == current_checksum)
		return;

	if (last_checksum.empty( ))
	{
		remove_all(dir);
		create_directories(dir);
	}

	std::ofstream file(checksum_file);
	file << current_checksum;
#endif

	lazy_writer__.reserve(data__.size( ) * 2);

	for (auto& [class_name, netvars]: data__)
	{
		// ReSharper disable CppInconsistentNaming
		// ReSharper disable CppTooWideScope
		constexpr auto __New_line = '\n';
		constexpr auto __Tab = '	';
		// ReSharper restore CppTooWideScope
		// ReSharper restore CppInconsistentNaming

		auto header = lazy_file_writer(dir / (class_name + "_h"));
		auto source = lazy_file_writer(dir / (class_name + "_cpp"));

		source << format("#include \"{}.h\"", class_name) << __New_line;
		source << __New_line;
		static const auto netvars_header_path = []
		{
			const auto cpp_path = CHEAT_CURRENT_FILE_PATH;
			auto str = string(cpp_path._Unchecked_begin( ), cpp_path.rfind('.'));
			ranges::replace(str, '\\', '/');
			return str += ".h";
		}( );
		source << format("#include \"{}\"", netvars_header_path) << __New_line;
		source << __New_line;
		source << "using namespace cheat;" << __New_line;
		source << "using namespace csgo;" << __New_line;
		source << __New_line;

		for (const auto& [netvar_name, netvar_data]: netvars)
		{
#ifdef CHEAT_NETVARS_DUMP_STATIC_OFFSET
			const auto netvar_offset = netvar_info::offset.ptree_get(netvar_data);
#endif
			auto netvar_type = netvar_info::type.ptree_get(netvar_data);
			const auto netvar_type_pointer = netvar_type.ends_with('*');
			if (netvar_type_pointer)
				netvar_type.pop_back( );

			const auto netvar_ret_char = netvar_type_pointer ? '*' : '&';

			header << format("{}{} {}( );", netvar_type, netvar_ret_char, netvar_name) << __New_line;
			source << format("{}{} {}::{}( )", netvar_type, netvar_ret_char, class_name, netvar_name) << __New_line;
			source << '{' << __New_line;
			source << "#ifdef CHEAT_NETVARS_UPDATING" << __New_line;
			source << __Tab << format("return {}({}*)nullptr;", netvar_type_pointer ? "" : "*", netvar_type) << __New_line;
			source << "#else" << __New_line;
#ifdef CHEAT_NETVARS_DUMP_STATIC_OFFSET
			source  << __Tab<< format("auto addr = utl::address(this).add({});", netvar_offset) << __New_line;
#else
			source << __Tab << format("static const auto offset = netvars::get_ptr( )->at(\"{}\");",
									  format("{}{}{}", class_name, PATH_DEFAULT_SEPARATOR, netvar_name)) << __New_line;
			source << __Tab << "auto addr = utl::address(this).add(offset);" << __New_line;
#endif
			source << __Tab << format("return addr.{}<{}>( );", netvar_type_pointer ? "raw" : "ref", netvar_type) << __New_line;
			source << "#endif" << __New_line;
			source << '}' << __New_line;
		}

		lazy_writer__.push_back(move(header));
		lazy_writer__.push_back(move(source));
	}

	if (last_checksum.empty( ))
	{
		//write all without waiting
		lazy_writer__.clear();
	}

#ifdef CHEAT_HAVE_CONSOLE
	console::get_ptr( )->write_line("Netvars classes generation done");
#endif
}

int netvars::at(const string_view& path) const
{
	auto& child = data__.get_child(string(path));

	//#ifdef _DEBUG
	//	auto& child1 = const_cast<property_tree::ptree&>(child);
	//	BOOST_ASSERT_MSG(netvar_info::in_use.ptree_get(child1) == false, "Netvar already used!");
	//	netvar_info::in_use.ptree_put(child1, true);
	//#endif

	return netvar_info::offset.ptree_get(child);
}

netvars::lazy_file_writer::~lazy_file_writer( )
{
	if (file__.empty( ))
		return;

	const auto str = this->str( );
	if (str.empty( ))
		return;

	std::ofstream ofs(file__);
	if (!ofs)
		return;

	ofs << str;
}

netvars::lazy_file_writer::lazy_file_writer(filesystem::path&& file): file__(utl::move(file))
{
}

netvars::lazy_file_writer::lazy_file_writer(lazy_file_writer&& other) noexcept
{
	file__ = utl::move(other.file__);
	*static_cast<std::ostringstream*>(this) = static_cast<std::ostringstream&&>(other);
}
