#include "netvars.h"

#include "cheat/core/console.h"
#include "cheat/core/csgo interfaces.h"

#include "cheat/sdk/IBaseClientDll.hpp"
#include "cheat/sdk/IVEngineClient.hpp"
#include "cheat/sdk/entity/C_BaseEntity.h"
#include "cheat/sdk/entity/C_CSPlayer.h"

#if defined(_DEBUG) || defined(CHEAT_NETVARS_UPDATING)
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

netvars::~netvars( )
{
}

netvars::netvars( )
{
}

static std::string _Str_to_lower(const std::string_view& str)
{
	std::string ret;
	ret.reserve(str.size( ));
	for (const int c: str)
		ret += std::tolower(c);
	return ret;
}

static bool _Save_netvar_allowed(const char* name)
{
	for (auto c = name[0]; c != '\0'; c = *(++name))
	{
		if (c == '.')
			return false;
	}
	return true;
}

static bool _Save_netvar_allowed(const std::string_view& name)
{
	return ranges::find(name, '.') == name.end( );
}

template <typename Nstr>
static bool _Save_netvar(nlohmann::json& storage, Nstr&& name, int offset, [[maybe_unused]] std::string&& type)
{
	auto path = (std::string(std::forward<Nstr>(name)));

	auto&& [entry, added] = storage.emplace((std::move(path)), nlohmann::json{ });
	if (added == false)
	{
#ifdef CHEAT_NETVARS_RESOLVE_TYPE
		if (type != "void*")
		{
			auto& type_obj = entry->at("type").get_ref<std::string&>( );
			if (type_obj != type)
				type_obj = std::move(type);
		}
#endif
	}
	else
	{
		*entry =
		{
			{"offset", offset},
#ifdef CHEAT_NETVARS_RESOLVE_TYPE
			{"type", std::move(type)}
#endif
		};
	}

	return added;
}

template <typename Nstr>
static std::pair<nlohmann::json::iterator, bool> _Add_child_class(nlohmann::json& storage, Nstr&& name)
{
	std::string class_name;
	if (name[0] == 'C' && name[1] != '_')
	{
		const auto name1 = std::string_view(name);
		//internal csgo classes looks like C_***
		//same classes in shared code look like C***
		class_name.reserve(name1.size( ) + 1);
		class_name += "C_";
		class_name.append(name1.begin( ) + 1, name1.end( ));
	}
	else
	{
		class_name = std::string(std::forward<Nstr>(name));
		runtime_assert(!class_name.starts_with("DT_"));
	}

	return storage.emplace(std::move(class_name), nlohmann::json{ });
}

static std::string _Array_type(const std::string_view& type, size_t size)
{
	return format("std::array<{}, {}>", type, size);
}

static std::string _Netvar_vec_type(const std::string_view& name)
{
	// ReSharper disable once CppTooWideScopeInitStatement
	const auto is_qangle = [&]
	{
		if (name.starts_with("m_ang"))
			return true;
		auto lstr = _Str_to_lower(name);
		return lstr.find("angles") != lstr.npos;
	};

	return (std::isdigit(name[0]) || !is_qangle( ) ? "Vector" : "QAngle");
}

static std::string _Netvar_int_type(std::string_view name)
{
	if (!std::isdigit(name[0]) && name.starts_with("m_"))
	{
		name.remove_prefix(2);
		const auto is_upper = [&](size_t i)
		{
			return name.size( ) > i && std::isupper(name[i]);
		};
		(void)is_upper;
		if (is_upper(1))
		{
			if (name.starts_with('b'))
				return "bool";
			if (name.starts_with('c'))
				return "uint8_t";
			if (name.starts_with('h'))
				return "CBaseHandle";
		}
		else if (is_upper(2))
		{
			if (name.starts_with("un"))
				return "uint32_t";
			if (name.starts_with("ch"))
				return "uint8_t";
			if (name.starts_with("fl") && _Str_to_lower(name).find("time") != std::string::npos) //m_flSimulationTime int ???
				return "float";
		}
		else if (is_upper(3))
		{
			if (name.starts_with("clr"))
				return "Color"; //not sure
		}
	}
	return "int32_t";
}

static std::string _Recv_prop_type(const RecvProp& prop)
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
			return "Vector2D"; //3d vector. z unused
		case DPT_String:
			return "char*";
		case DPT_Array:
		{
			const auto& prev_prop = *std::prev(std::addressof(prop));
			runtime_assert(std::string_view(prev_prop.m_pVarName).ends_with("[0]"));
			const auto type = _Recv_prop_type(prev_prop);
			return _Array_type(type, prop.m_nElements);
		}
		case DPT_DataTable:
		{
			runtime_assert("Data table type must be manually resolved!");
			return "void*";
		}
		case DPT_Int64:
			return "int64_t";
		default:
		{
			runtime_assert("Unknown recv prop type");
			return "void*";
		}
	}
}

[[maybe_unused]]
static std::string _Datamap_field_type(const typedescription_t& field)
{
	switch (field.fieldType)
	{
		case FIELD_VOID:
			return "void*";
		case FIELD_FLOAT:
			return "float";
		case FIELD_STRING:
			return "char*"; //std::string_t at real
		case FIELD_VECTOR:
			return _Netvar_vec_type(field.fieldName);
		case FIELD_QUATERNION:
		{
			//return "Quaterion";
			runtime_assert("Quaterion field detected");
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
			return "Color";
		case FIELD_EMBEDDED:
		{
			runtime_assert("Embedded field detected");
			return "void*";
		}
		case FIELD_CUSTOM:
		{
			runtime_assert("Custom field detected");
			return "void*";
		}
		case FIELD_CLASSPTR:
			return "C_BaseEntity*";
		case FIELD_EHANDLE:
			return "CBaseHandle";
		case FIELD_EDICT:
		{
			//return "edict_t*";
			runtime_assert("Edict field detected");
			return "void*";
		}
		case FIELD_POSITION_VECTOR:
			return "Vector";
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
			runtime_assert("Inputvar field detected");
			return "void*";
		}
		case FIELD_FUNCTION:
		{
			runtime_assert("Function detected");
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
			runtime_assert("Interval field detected");
			return "void*";
		}
		case FIELD_MODELINDEX:
		case FIELD_MATERIALINDEX:
			return "int32_t";
		case FIELD_VECTOR2D:
			return "Vector2D";
		default:
		{
			runtime_assert("Unknown datamap field type");
			return "void*";
		}
	}
}

static void _Store_recv_props(nlohmann::json& root_tree, nlohmann::json& tree, const RecvTable* recv_table, int offset)
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
		return prop.m_pVarName == std::string_view("baseclass");
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
		auto back  = std::prev(raw_props.end( ));

		if (prop_is_base_class(*front))
			++front;
		if (prop_is_length_proxy(*back))
			--back;

		return std::span(front, std::next(back));
	}( );

	for (auto itr = props.begin( ); itr != props.end( ); ++itr)
	{
		// ReSharper disable once CppUseStructuredBinding
		const auto& prop = *itr;
		runtime_assert(prop.m_pVarName != nullptr);
		const auto prop_name = std::string_view(prop.m_pVarName);

		if (!_Save_netvar_allowed(prop_name))
			continue;

		const auto real_prop_offset = offset + prop.m_Offset;

		if (prop_name.rfind(']') != prop_name.npos)
		{
			if (prop_name.ends_with("[0]"))
			{
				const auto real_prop_name = std::string_view(prop_name.begin( ), std::prev(prop_name.end( ), 3));
				runtime_assert(!real_prop_name.ends_with(']'));
				auto array_size = std::optional<size_t>(1);

				// ReSharper disable once CppUseStructuredBinding
				for (const auto& p: std::span(std::next(itr), props.end( )))
				{
					if (const auto name = std::string_view(p.m_pVarName); name.starts_with(real_prop_name))
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
					std::string netvar_type;
					if (*array_size == 3 && (std::isupper(real_prop_name[5]) && real_prop_name.starts_with("m_")))
					{
						auto prefix = real_prop_name.substr(2, 3);
						if (prop.m_RecvType == DPT_Float)
						{
							if (prefix == "ang")
								netvar_type = "QAngle";
							else if (prefix == "vec")
								netvar_type = "Vector";
						}
						else if (prop.m_RecvType == DPT_Int)
						{
							if (prefix == "uch")
								netvar_type = "Color";
						}
					}

					if (netvar_type.empty( ))
					{
						auto type   = _Recv_prop_type(prop);
						netvar_type = _Array_type(type, *array_size);
					}
#else
					std::string netvar_type;
#endif
					_Save_netvar(tree, real_prop_name, real_prop_offset, std::move(netvar_type));
					itr += *array_size - 1;
				}
			}
			continue;
		}
#if 0
		optional<size_t> array_size;
		if (std::isdigit(prop_name[0]))
		{
			runtime_assert(prop_name[0] == '0');

			const auto part = props.subspan(i + 1);
			const auto array_end_itr = ranges::find_if_not(part, [](const RecvProp& rp) { return std::isdigit(rp.m_pVarName[0]); });
			const auto array_end_num = std::distance(part.begin( ), array_end_itr);

			array_size = array_end_num - i + 1;
			i += array_end_num;

			static const std::string props_array = "m_PropsArray";
			prop_name = props_array;
		}

		if (prop.m_ArrayLengthProxy != nullptr)
		{
			continue;
		}
#else
		runtime_assert(!prop_is_length_proxy(prop));
		runtime_assert(!std::isdigit(prop_name[0]));
#endif

		if (prop.m_RecvType != DPT_DataTable)
		{
#ifdef CHEAT_NETVARS_RESOLVE_TYPE
			auto netvar_type = _Recv_prop_type(prop);
#else
			std::string netvar_type;
#endif
			_Save_netvar(tree, prop_name, real_prop_offset, std::move(netvar_type));
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
					runtime_assert(array_begin->m_pVarName[0] == '0');
				}
				runtime_assert(array_begin != child_props.end( ));

#ifdef _DEBUG
				// ReSharper disable once CppUseStructuredBinding
				for (const auto& rp: std::span(array_begin + 1, child_props.end( )))
				{
					runtime_assert(std::isdigit(rp.m_pVarName[0]));
					runtime_assert(rp.m_RecvType == array_begin->m_RecvType);
				}
#endif

#ifdef CHEAT_NETVARS_RESOLVE_TYPE
				const auto  array_size = std::distance(array_begin, child_props.end( ));
				std::string netvar_type;
#endif
				if (array_begin->m_RecvType != DPT_DataTable)
				{
#ifdef CHEAT_NETVARS_RESOLVE_TYPE
					netvar_type = _Recv_prop_type(*array_begin);
#endif
				}
				else
				{
					const auto  child_table_name = std::string_view(child_table->m_pNetTableName);
					std::string child_table_unique_name;
					if (prop_name != child_table_name)
					{
						child_table_unique_name = child_table_name;
					}
					else
					{
						constexpr auto unique_str = std::string_view("_t");
						child_table_unique_name.reserve(child_table_name.size( ) + unique_str.size( ));
						child_table_unique_name.append(child_table_name);
						child_table_unique_name.append(unique_str);
					}
					auto [new_tree, added] = _Add_child_class(root_tree, child_table_unique_name);
					if (!added)
						continue;
#ifdef CHEAT_NETVARS_RESOLVE_TYPE
					netvar_type = std::move(child_table_unique_name);
#endif
					_Store_recv_props(root_tree, *new_tree, array_begin->m_pDataTable, /*real_prop_offset*/0);
				}

#ifdef CHEAT_NETVARS_RESOLVE_TYPE
				auto netvar_type_array = _Array_type(netvar_type, array_size);
#else
				std::string netvar_type_array;
#endif
				_Save_netvar(tree, prop_name, real_prop_offset, std::move(netvar_type_array));
			}
			else
			{
				runtime_assert("Unknown netvar type");
			}
		}
	}
}

[[maybe_unused]]
static void _Iterate_client_class(nlohmann::json& root_tree, ClientClass* root_class)
{
	for (auto client_class = root_class; client_class != nullptr; client_class = client_class->pNext)
	{
		const auto recv_table = client_class->pRecvTable;
		if (!recv_table || recv_table->props.empty( ))
			continue;

		auto [new_tree, added] = _Add_child_class(root_tree, client_class->pNetworkName);
		runtime_assert(added == true);

		_Store_recv_props(root_tree, *new_tree, recv_table, 0);

		if (new_tree->empty( ))
			root_tree.erase(new_tree);
	}
}

static void _Store_datamap_props(nlohmann::json& tree, datamap_t* map)
{
	// ReSharper disable once CppUseStructuredBinding
	for (auto& desc: map->data)
	{
		if (desc.fieldType == FIELD_EMBEDDED)
		{
			if (desc.TypeDescription != nullptr)
			{
				runtime_assert("Embedded datamap detected");
			}
		}
		else if (desc.fieldName != nullptr)
		{
			const auto field_name = std::string_view(desc.fieldName);

			if (!_Save_netvar_allowed(field_name))
				continue;

			const auto offset = desc.fieldOffset[TD_OFFSET_NORMAL];

#ifdef CHEAT_NETVARS_RESOLVE_TYPE
			auto field_type = _Datamap_field_type(desc);
#else
			std::string field_type;
#endif
			_Save_netvar(tree, field_name, offset, std::move(field_type));
		}
	}
}

[[maybe_unused]]
static void _Iterate_datamap(nlohmann::json& root_tree, datamap_t* root_map)
{
	for (auto map = root_map; map != nullptr; map = map->baseMap)
	{
		if (map->data.empty( ))
			continue;

		auto&& [tree, added] = _Add_child_class(root_tree, map->dataClassName);

		_Store_datamap_props(*tree, map);

		if (added && tree->empty( ))
			root_tree.erase(tree);
	}
}

bool netvars::Do_load( )
{
#if defined(CHEAT_NETVARS_DUMPER_DISABLED)
	return false;
#else

#ifdef _DEBUG
	data__.clear( );
#endif

	const auto interfaces = csgo_interfaces::get_ptr( );

	_Iterate_client_class(data__, interfaces->client->GetAllClasses( ));

	const auto baseent = _Vtable_pointer<C_BaseEntity>("client.dll");

	_Iterate_datamap(data__, baseent->GetDataDescMap( ));
	_Iterate_datamap(data__, baseent->GetPredictionDescMap( ));

	return true;

#endif
}

void netvars::On_load( )
{
#if defined(CHEAT_NETVARS_RESOLVE_TYPE) && !defined(CHEAT_NETVARS_DUMPER_DISABLED)

	const auto info = this->Dump_netvars_( );
	this->Generate_classes_(info);

#endif
}

static constexpr int  _Json_indent = 4;
static constexpr char _Json_filler = ' ';

netvars::dump_info netvars::Dump_netvars_( )
{
	static const auto netvars_dump_dir = std::filesystem::path(CHEAT_DUMPS_DIR) / L"netvars";
	const auto        dirs_created     = create_directories(netvars_dump_dir);

	constexpr auto get_file_name = []
	{
		std::string version = csgo_interfaces::get_ptr( )->engine->GetProductVersionString( );
		ranges::replace(version, '.', '_');
		version.append(".json");
		return version;
	};

	const auto netvars_dump_file = netvars_dump_dir / get_file_name( );
	const auto file_exists       = !dirs_created && exists(netvars_dump_file);

	dump_info info;

	if (!file_exists)
	{
		std::ofstream(netvars_dump_file) << std::setw(_Json_indent) << std::setfill(_Json_filler) << data__;
		info = dump_info::created;
		CHEAT_CONSOLE_LOG("Netvars dump done");
	}
	else
	{
		std::ostringstream test_to_write;
		test_to_write << std::setw(_Json_indent) << std::setfill(_Json_filler) << data__;

		if ((nstd::checksum(test_to_write) == nstd::checksum(netvars_dump_file)))
		{
			info = dump_info::skipped;
			CHEAT_CONSOLE_LOG("Netvars dump skipped");
		}
		else
		{
			std::ofstream(netvars_dump_file) << nstd::as_string(test_to_write);
			info = dump_info::updated;
			CHEAT_CONSOLE_LOG("Netvars dump updated");
		}
	}

	return info;
}

void netvars::Generate_classes_(dump_info info)
{
#ifndef CHEAT_NETVARS_RESOLVE_TYPE
	(void)this;
#else

	if (info == dump_info::skipped)
		return;

	static const auto dir = std::filesystem::path(CHEAT_IMPL_DIR) / L"sdk" / L"generated";

	remove_all(dir);
	create_directories(dir);

	lazy_writer__.reserve(data__.size( ) * 2);
	for (auto& [class_name,netvars]: data__.items( ))
	{
		// ReSharper disable CppInconsistentNaming
		// ReSharper disable CppTooWideScope
		constexpr auto __New_line = '\n';
		constexpr auto __Tab      = '	';
		// ReSharper restore CppTooWideScope
		// ReSharper restore CppInconsistentNaming

		auto header = lazy_file_writer(dir / (class_name + "_h"));
		auto source = lazy_file_writer(dir / (class_name + "_cpp"));

		source << std::format("#include \"{}.h\"", class_name) << __New_line;
		source << __New_line;
		static const auto netvars_header_path = []
		{
			const auto cpp_path = CHEAT_CURRENT_FILE_PATH;
			auto       str      = std::string(cpp_path._Unchecked_begin( ), cpp_path.rfind('.'));
			ranges::replace(str, '\\', '/');
			return str += ".h";
		}( );
		source << format("#include \"{}\"", netvars_header_path) << __New_line;
		source << __New_line;
		source << "using namespace cheat;" << __New_line;
		source << "using namespace utl;" << __New_line;
		source << "using namespace csgo;" << __New_line;
		source << __New_line;

		for (auto& [netvar_name,netvar_data]: netvars.items( ))
		{
#ifdef CHEAT_NETVARS_DUMP_STATIC_OFFSET
			const auto netvar_offset = netvar_info::offset.get(netvar_data);
#endif
			std::string_view netvar_type         = netvar_data.at("type").get_ref<std::string&>( );
			const auto       netvar_type_pointer = netvar_type.ends_with('*');
			if (netvar_type_pointer)
				netvar_type.remove_suffix(1);

			const auto netvar_ret_char = netvar_type_pointer ? '*' : '&';

			header << format("{}{} {}( );", netvar_type, netvar_ret_char, netvar_name) << __New_line;
			source << format("{}{} {}::{}( )", netvar_type, netvar_ret_char, class_name, netvar_name) << __New_line;
			source << '{' << __New_line;
			source << "#ifdef CHEAT_NETVARS_UPDATING" << __New_line;
			source << __Tab << format("return {}({}*)nullptr;", netvar_type_pointer ? "" : "*", netvar_type) << __New_line;
			source << "#else" << __New_line;
#ifdef CHEAT_NETVARS_DUMP_STATIC_OFFSET
			source  << __Tab<< format("auto addr = address(this).add({});", netvar_offset) << __New_line;
#else
			source << __Tab << std::format(
										   "static const auto offset = netvars::get_ptr( )->at(\"{}\", \"{}\");",
										   class_name, netvar_name
										  ) << __New_line;
			source << __Tab << "auto addr = address(this).add(offset);" << __New_line;
#endif
			source << __Tab << format("return addr.{}<{}>( );", netvar_type_pointer ? "ptr" : "ref", netvar_type) << __New_line;
			source << "#endif" << __New_line;
			source << '}' << __New_line;
		}

		lazy_writer__.push_back(std::move(header));
		lazy_writer__.push_back(std::move(source));
	}

	if (info == dump_info::created)
	{
		//write all without waiting
		lazy_writer__.clear( );
	}

	CHEAT_CONSOLE_LOG("Netvars classes generation done");
#endif
}

int netvars::at(const std::string_view& table, const std::string_view& item) const
{
	for (auto& [table_stored,keys]: data__.items( ))
	{
		if (table_stored != table)
			continue;

		for (auto& [item_stored,data]: keys.items( ))
		{
			if (item_stored == item)
				return data.get<int>( );
		}

		runtime_assert(false, std::format(__FUNCTION__": item {} not found in table {}",item,table).c_str( ));
		return 0;
	}

	runtime_assert(false, std::format(__FUNCTION__": table {} not found",table).c_str( ));

	return 0;
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

netvars::lazy_file_writer::lazy_file_writer(std::filesystem::path&& file): file__(std::move(file))
{
}

netvars::lazy_file_writer::lazy_file_writer(lazy_file_writer&& other) noexcept
{
	file__                                  = std::move(other.file__);
	*static_cast<std::ostringstream*>(this) = static_cast<std::ostringstream&&>(other);
}
