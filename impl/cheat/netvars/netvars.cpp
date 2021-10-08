#include "netvars.h"
#include "config.h"

#include "cheat/core/services loader.h"
#include "cheat/core/console.h"
#include "cheat/core/csgo interfaces.h"
#include "cheat/core/csgo modules.h"

#include "cheat/sdk/IBaseClientDll.hpp"
#include "cheat/sdk/IVEngineClient.hpp"
#include "cheat/sdk/entity/C_BaseEntity.h"

#include "nstd/checksum.h"
#include "nstd/unistring.h"

#include <robin_hood.h>

#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <ranges>
#include <set>

using namespace cheat;
using namespace detail;
using namespace csgo;

class lazy_file_writer final : public std::ostringstream
{
public:
	~lazy_file_writer() override
	{
		if (file_.empty( ))
			return;

		const auto str = this->view( );
		if (str.empty( ))
			return;

		std::ofstream ofs(file_);
		if (!ofs)
			return;

		ofs << str;
	}

	lazy_file_writer(std::filesystem::path&& file)
		: file_(std::move(file))
	{
	}

	lazy_file_writer(lazy_file_writer&& other) noexcept
	{
		*this = std::move(other);
	}

	lazy_file_writer& operator=(lazy_file_writer&& other) noexcept
	{
		std::swap(file_, other.file_);
		std::swap<std::ostringstream>(*this, other);
		return *this;
	}

private:
	std::filesystem::path file_;
};

struct netvars::hidden
{
	using storage_type = nlohmann::ordered_json;
	using lazy_writer_type = std::vector<lazy_file_writer>;

	lazy_writer_type lazy_writer;
	storage_type storage;
};

netvars::~netvars() = default;

netvars::netvars()
{
	hidden_ = std::make_unique<hidden>( );
	this->wait_for_service<csgo_interfaces>( );
}

static std::string _Str_to_lower(const std::string_view& str)
{
	std::string ret;
	ret.reserve(str.size( ));
	for (const auto c : str)
		ret += static_cast<char>(std::tolower(c));
	return ret;
}

[[maybe_unused]]
static bool _Save_netvar_allowed(const char* name)
{
	for (auto c = name[0]; c != '\0'; c = *++name)
	{
		if (c == '.')
			return false;
	}
	return true;
}

static bool _Save_netvar_allowed(const std::string_view& name)
{
	return std::ranges::any_of(name, [](char c) { return c == '.'; });
}

struct netvar_type_holder : std::string
{
	netvar_type_holder() = default;

	netvar_type_holder(const std::string_view& sv)
		: std::string(sv.begin( ), sv.end( ))
	{
	}

	netvar_type_holder(std::string&& sv)
		: std::string(std::move(sv))
	{
	}

	netvar_type_holder(const char*) = delete;
};

template <typename Nstr>
static bool _Save_netvar(netvars::hidden::storage_type& storage, Nstr&& name, int offset, [[maybe_unused]] netvar_type_holder&& type)
{
	auto path = std::string(std::forward<Nstr>(name));

	auto&& [entry, added] = storage.emplace(std::move(path), netvars::hidden::storage_type{});
	if (added == false)
	{
#ifdef CHEAT_NETVARS_RESOLVE_TYPE
		if (type != nstd::type_name<void*>)
		{
			auto& type_obj = entry->at("type").get_ref<std::string&>( );
			if (type_obj != type)
				type_obj.assign(((std::string&&)type));
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
static std::pair<netvars::hidden::storage_type::iterator, bool> _Add_child_class(netvars::hidden::storage_type& storage, Nstr&& name)
{
	std::string class_name;
	if (name[0] == 'C' && name[1] != '_')
	{
		const auto name1 = std::string_view(name);
		//internal csgo classes looks like C_***
		//same classes in shared code look like C***
		class_name.reserve(name1.size( ) + 1);
		class_name += "C_";
		class_name.append(std::next(name1.begin( )), name1.end( ));
	}
	else
	{
		class_name = std::string(std::forward<Nstr>(name));
		runtime_assert(!class_name.starts_with("DT_"));
	}

	return storage.emplace(std::move(class_name), netvars::hidden::storage_type::value_type{});
}

static netvar_type_holder _As_std_array_type(const std::string_view& type, size_t size)
{
	return std::format("std::array<{}, {}>", type, size);
}

static netvar_type_holder _Netvar_vec_type(const std::string_view& name)
{
	const auto is_qangle = [&]
	{
		if (name.starts_with("m_ang"))
			return true;
		auto lstr = _Str_to_lower(name);
		return lstr.find("angles") != lstr.npos;
	};

	return std::isdigit(name[0]) || !is_qangle( ) ? nstd::type_name<Vector> : nstd::type_name<QAngle>;
}

static netvar_type_holder _Netvar_int_type(std::string_view name)
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
			if (name.starts_with("fl") && _Str_to_lower(name).find("time") != std::string::npos) //m_flSimulationTime int ???
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

static netvar_type_holder _Recv_prop_type(const RecvProp& prop)
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

[[maybe_unused]]
static netvar_type_holder _Datamap_field_type(const typedescription_t& field)
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

static void _Store_recv_props(netvars::hidden::storage_type& root_tree, netvars::hidden::storage_type& tree, const RecvTable* recv_table, int offset)
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
				for (const auto& p : std::span(std::next(itr), props.end( )))
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
					netvar_type_holder netvar_type;
#ifdef CHEAT_NETVARS_RESOLVE_TYPE
					if (*array_size == 3 && (std::isupper(real_prop_name[5]) && real_prop_name.starts_with("m_")))
					{
						auto prefix = real_prop_name.substr(2, 3);
						if (prop.m_RecvType == DPT_Float)
						{
							if (prefix == "ang")
								netvar_type = nstd::type_name<QAngle>;
							else if (prefix == "vec")
								netvar_type = nstd::type_name<Vector>;
						}
						else if (prop.m_RecvType == DPT_Int)
						{
							if (prefix == "uch")
								netvar_type = nstd::type_name<Color>;
						}
					}

					if (netvar_type.empty( ))
					{
						auto type   = _Recv_prop_type(prop);
						netvar_type = _As_std_array_type(type, *array_size);
					}
#endif
					_Save_netvar(tree, real_prop_name, real_prop_offset, std::move(netvar_type));
					itr += *array_size - 1;
				}
			}
			continue;
		}
#if 0
		optional<size_t> array_size;
		if(std::isdigit(prop_name[0]))
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

		if(prop.m_ArrayLengthProxy != nullptr)
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
			const auto child_table  = prop.m_pDataTable;
			const auto& child_props = child_table->props;
			if (!child_table || child_props.empty( ))
				continue;

			if (table_is_data_table(*child_table))
			{
				_Store_recv_props(root_tree, tree, child_table, real_prop_offset);
			}
			else if (table_is_array(*child_table))
			{
				auto array_begin = child_props.
#if _ITERATOR_DEBUG_LEVEL >= 1
					_Unchecked_begin( )
#else
						begin( );
#endif
				if (prop_is_length_proxy(*array_begin))
				{
					++array_begin;
					runtime_assert(array_begin->m_pVarName[0] == '0');
				}
				runtime_assert(array_begin != child_props.end( ));

#ifdef _DEBUG
				// ReSharper disable once CppUseStructuredBinding
				for (const auto& rp : std::span(std::next(array_begin), child_props.end( )))
				{
					runtime_assert(std::isdigit(rp.m_pVarName[0]));
					runtime_assert(rp.m_RecvType == array_begin->m_RecvType);
				}
#endif

#ifdef CHEAT_NETVARS_RESOLVE_TYPE
				const auto array_size = std::distance(array_begin, child_props.end( ));
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
					const auto child_table_name = std::string_view(child_table->m_pNetTableName);
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
				auto netvar_type_array = _As_std_array_type(netvar_type, array_size);
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
static void _Iterate_client_class(netvars::hidden::storage_type& root_tree, ClientClass* root_class)
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

static void _Store_datamap_props(netvars::hidden::storage_type& tree, datamap_t* map)
{
	// ReSharper disable once CppUseStructuredBinding
	for (auto& desc : map->data)
	{
		if (desc.fieldType == FIELD_EMBEDDED)
		{
			if (desc.TypeDescription != nullptr)
				runtime_assert("Embedded datamap detected");
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
static void _Iterate_datamap(netvars::hidden::storage_type& root_tree, datamap_t* root_map)
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

//#define CHEAT_SOLUTION_DIR NSTD_STRINGIZE_RAW((VS_SolutionDir))
//#define CHEAT_DUMPS_DIR /*CHEAT_OUTPUT_DIR*/CHEAT_SOLUTION_DIR NSTD_STRINGIZE_RAW(.out\dumps\)
//#define CHEAT_ROOT_DIR CHEAT_SOLUTION_DIR NSTD_STRINGIZE_RAW(impl\)
//#define CHEAT_SOURCE_DIR CHEAT_ROOT_DIR NSTD_STRINGIZE_RAW(cheat\)
//
//// ReSharper disable CppInconsistentNaming
//static const auto CHEAT_NETVARS_GENERATED_CLASSES_DIR = std::filesystem::path(R"((C:\Users\owner\Documents\programming\Projects\git\dvvn\from-darkness\))" R"(impl\)" R"(cheat\)") / L"sdk" / L"generated";
//static const auto CHEAT_NETVARS_DUMP_DIR              = std::filesystem::path(CHEAT_DUMPS_DIR) / L"netvars";
//
//static constexpr int CHEAT_NETVARS_JSON_INDENT  = 4;
//static constexpr char CHEAT_NETVARS_JSON_FILLER = ' ';
//// ReSharper restore CppInconsistentNaming

enum class dump_info :uint8_t
{
	unset
  , skipped
  , created
  , updated
};

#define STRINGIZE_PATH(_PATH_) \
	_CONCAT(LR,_STRINGIZE(##(_PATH_)##))

[[maybe_unused]]
static dump_info _Dump_netvars(const netvars::hidden::storage_type& netvars_data)
{
	const std::filesystem::path dumps_dir = STRINGIZE_PATH(CHEAT_NETVARS_DUMPS_DIR);

	const auto dirs_created = std::filesystem::create_directories(dumps_dir);

	constexpr auto get_file_name = []()-> std::filesystem::path
	{
		std::string version = csgo_interfaces::get_ptr( )->engine->GetProductVersionString( );
		std::ranges::replace(version, '.', '_');
		version.append(".json");
		return version;
	};

	const auto netvars_dump_file = dumps_dir / get_file_name( );
	const auto file_exists       = !dirs_created && exists(netvars_dump_file);

	if (!file_exists)
	{
		std::ofstream(netvars_dump_file) << std::setw(CHEAT_NETVARS_DUMP_FILE_INDENT) << std::setfill(CHEAT_NETVARS_DUMP_FILE_FILLER) << netvars_data;
		CHEAT_CONSOLE_LOG("Netvars dump done");
		return dump_info::created;
	}

	//------

	std::ostringstream netvars_data_as_text;
	netvars_data_as_text << std::setw(CHEAT_NETVARS_DUMP_FILE_INDENT) << std::setfill(CHEAT_NETVARS_DUMP_FILE_FILLER) << netvars_data;

	if (nstd::checksum(netvars_dump_file) != nstd::checksum(netvars_data_as_text))
	{
		std::ofstream(netvars_dump_file) << netvars_data_as_text.view( );
		CHEAT_CONSOLE_LOG("Netvars dump updated");
		return dump_info::updated;
	}

	CHEAT_CONSOLE_LOG("Netvars dump skipped");
	return dump_info::skipped;
}

struct include_name : std::string
{
	template <typename T>
	include_name(T&& name, bool global)
		: std::string(std::forward<T>(name)), global(global)
	{
	}

	bool global;
};

[[maybe_unused]]
static void _Generate_classes(dump_info info, const netvars::hidden::storage_type& netvars_data, netvars::hidden::lazy_writer_type& lazy_writer)
{
	const std::filesystem::path generated_classes_dir = STRINGIZE_PATH(CHEAT_NETVARS_GENERATED_DIR);

	if (info == dump_info::skipped || info == dump_info::updated)
	{
		if (!exists(generated_classes_dir))
		{
			info = dump_info::created;
			goto _CREATE;
		}
		if (is_empty(generated_classes_dir))
		{
			info = dump_info::created;
			goto _WORK;
		}

		const auto net_classes = [&]
		{
			auto data = robin_hood::unordered_set<std::string_view>( );
			data.reserve(netvars_data.size( ));
			for (auto& [class_name, netvars] : netvars_data.items( ))
				data.emplace(class_name);
			return data;
		}( );

		for (auto& entry : std::filesystem::directory_iterator(generated_classes_dir))
		{
			const auto name      = entry.path( ).filename( ).string( );
			const auto real_size = name.rfind('_'); //erase _h _cpp

			const auto valid_name = std::string_view(name._Unchecked_begin( ), real_size);
			if (!net_classes.contains(valid_name))
			{
				info = dump_info::updated;
				goto _REMOVE;
			}
		}

		if (info == dump_info::skipped)
			return;
	}

_REMOVE:
	std::filesystem::remove_all(generated_classes_dir);
_CREATE:
	std::filesystem::create_directories(generated_classes_dir);
_WORK:

	lazy_writer.reserve(netvars_data.size( ) * 2);
	for (auto& [CLASS_NAME, NETVARS] : netvars_data.items( ))
	{
		// ReSharper disable CppInconsistentNaming
		// ReSharper disable CppTooWideScope
		constexpr auto __New_line = '\n';
		constexpr auto __Tab      = '	';
		// ReSharper restore CppTooWideScope
		// ReSharper restore CppInconsistentNaming

		auto header = lazy_file_writer(generated_classes_dir / (CLASS_NAME + _STRINGIZE(CHEAT_NETVARS_GENERATED_HEADER_POSTFIX)));
		auto source = lazy_file_writer(generated_classes_dir / (CLASS_NAME + _STRINGIZE(CHEAT_NETVARS_GENERATED_SOURCE_POSTFIX)));

		const auto source_add_include = [&source](const std::string_view& file_name, bool global = false)
		{
			source <<
					"#include "
					<< (global ? '<' : '"')
					<< file_name
					<< (global ? '>' : '"')
					<< __New_line;
		};

		const auto source_add_dynamic_includes = [&]
		{
#if !defined(CHEAT_NETVARS_RESOLVE_TYPE)
			runtime_assert("Unable to get dynamic includes!");
#else

			//auto includes = robin_hood::unordered_set<include_name, robin_hood::hash<std::string>>( );
			auto includes = std::set<include_name>( );

			const auto dir_path = std::filesystem::path(STRINGIZE_PATH(CHEAT_CSGO_SDK_DIR));
			for (auto& [netvar_name, netvar_data] : NETVARS.items( ))
			{
				const auto netvar_type = nstd::drop_namespaces(netvar_data.at("type").get_ref<const std::string&>( ));

				if (netvar_type.starts_with("std"))
				{
					auto decayed = nstd::drop_namespaces(netvar_type);
					if (decayed.ends_with('<'))
						decayed.remove_suffix(1);
					includes.emplace(decayed, true);
				}
				else if (netvar_type.starts_with("cheat"))
				{
					std::wstring full_path;
					full_path.reserve(netvar_type.size( ));
					std::wstring_view path_to_file;

					for (auto itr = netvar_type.begin( ); itr != netvar_type.end( ); ++itr)
					{
						const auto c = *itr;
						if (c == ':')
						{
							++itr;
							full_path += '/';
							path_to_file = full_path;
						}
						else
						{
							full_path += c;
						}
					}

					const auto test_file_name = std::wstring_view(full_path).substr(path_to_file.size( ));
					const auto checked_folder = STRINGIZE_PATH(_CONCAT(VS_SolutionDir, impl\)) / std::filesystem::path(path_to_file);

					for (auto& entry : std::filesystem::directory_iterator(checked_folder))
					{
						if (!entry.is_regular_file( ))
							continue;

						auto full_file_name = std::wstring_view(entry.path( ).native( )).substr(checked_folder.native( ).size( ));
						if (!full_file_name.starts_with(test_file_name))
							continue;

						auto extension = full_file_name.substr(test_file_name.size( ));
						if (extension[0] != L'.')
							continue;

						if (extension[1] != L'h') //.cpp .cxx etc
							continue;

						std::string include;

						include.reserve(full_path.size( ) + extension.size( ));
						include.append(full_path._Unchecked_begin( ), full_path._Unchecked_end( ));
						include.append(extension._Unchecked_begin( ), extension._Unchecked_end( ));

						includes.emplace(std::move(include), false);
						break;
					}
				}
				else
				{
					runtime_assert(netvar_type.find("::") != netvar_type.npos, "Unknown namespace detected");

					if (netvar_type.ends_with("_t"))
						includes.emplace("cstdint", true);
				}
			}

			switch (includes.size( ))
			{
				case 0:
					return;
				case 1:
				{
					auto& first = *includes.begin( );
					source_add_include(first, first.global);
					break;
				}
				default:
				{
					const auto includes_adder = [&](bool global)
					{
						for (auto& in : includes)
						{
							if (in.global == global)
								source_add_include(in, global);
						}
					};

					includes_adder(false);
					includes_adder(true);

					break;
				}
			}

			source << __New_line;

#endif
		};

		source_add_include(CLASS_NAME + ".h");
		source_add_include("cheat/netvars/config.h");
		source << "#ifndef CHEAT_NETVARS_UPDATING" << __New_line;
		source_add_include("cheat/netvars/netvars.h");
		source_add_include("nstd/address.h", true);
		source << "#endif" << __New_line;
		source << __New_line;

		source_add_dynamic_includes( );

		//source << "using cheat::csgo::" << CLASS_NAME << ';' << __New_line;
		source << "using namespace cheat::csgo;" << __New_line;

		source << __New_line;

		for (auto& [NETVAR_NAME, NETVAR_DATA] : NETVARS.items( ))
		{
#ifdef CHEAT_NETVARS_DUMP_STATIC_OFFSET
			const auto netvar_offset = netvar_info::offset.get(NETVAR_DATA);
#endif
			std::string_view netvar_type   = NETVAR_DATA.at("type").get_ref<const std::string&>( );
			const auto netvar_type_pointer = netvar_type.ends_with('*');
			if (netvar_type_pointer)
				netvar_type.remove_suffix(1);

			const auto netvar_ret_char = netvar_type_pointer ? '*' : '&';

			// ReSharper disable once CppInconsistentNaming
			constexpr auto _Address_class = nstd::type_name<nstd::address>;

			header << std::format("{}{} {}( );", netvar_type, netvar_ret_char, NETVAR_NAME) << __New_line;
			source << std::format("{}{} {}::{}( )", netvar_type, netvar_ret_char, CLASS_NAME, NETVAR_NAME) << __New_line;
			source << '{' << __New_line;
			source << "#ifdef CHEAT_NETVARS_UPDATING" << __New_line;
			source << __Tab << std::format("return {}({}*)nullptr;", netvar_type_pointer ? "" : "*", netvar_type) << __New_line;
			source << "#else" << __New_line;
#ifdef CHEAT_NETVARS_DUMP_STATIC_OFFSET
			source << __Tab << format("auto addr = {}(this).add({});", _Address_class, netvar_offset) << __New_line;
#else
			source
					<< __Tab
					<< "static const auto offset = netvars::get_ptr( )->at"
					<< std::format("(\"{}\", \"{}\");", CLASS_NAME, NETVAR_NAME)
					<< __New_line;
			source << __Tab << "auto addr = " << _Address_class << "(this).add(offset);" << __New_line;
#endif
			source << __Tab << std::format("return addr.{}<{}>( );", netvar_type_pointer ? "ptr" : "ref", netvar_type) << __New_line;
			source << "#endif" << __New_line;
			source << '}' << __New_line;
		}

		lazy_writer.push_back(std::move(header));
		lazy_writer.push_back(std::move(source));
	}

	if (info == dump_info::created)
	{
		//write all without waiting
		lazy_writer.clear( );
	}

	CHEAT_CONSOLE_LOG("Netvars classes generation done");
}

service_base::load_result netvars::load_impl() noexcept
{
#if defined(CHEAT_NETVARS_DUMPER_DISABLED)

	CHEAT_SERVICE_SKIPPED
#else

	auto& data = hidden_->storage;

#ifdef _DEBUG
	data.clear( );
#endif

	const auto interfaces = csgo_interfaces::get_ptr( );

	_Iterate_client_class(data, interfaces->client->GetAllClasses( ));

	const auto baseent = csgo_modules::client.find_vtable<C_BaseEntity>( );

	_Iterate_datamap(data, baseent->GetDataDescMap( ));
	_Iterate_datamap(data, baseent->GetPredictionDescMap( ));

#if defined(CHEAT_NETVARS_RESOLVE_TYPE)

	const auto info = _Dump_netvars(hidden_->storage);
	_Generate_classes(info, hidden_->storage, hidden_->lazy_writer);

#endif
	CHEAT_SERVICE_LOADED

#endif
}

int netvars::at(const std::string_view& table, const std::string_view& item) const
{
	for (auto& [table_stored, keys] : hidden_->storage.items( ))
	{
		if (table_stored != table)
			continue;

		for (const auto& [item_stored, data] : keys.items( ))
		{
			if (item_stored == item)
				return data["offset"].get<int>( );
		}

		runtime_assert(false, std::format(__FUNCTION__": item {} not found in table {}", item, table).c_str( ));
		return 0;
	}

	runtime_assert(false, std::format(__FUNCTION__": table {} not found", table).c_str( ));
	return 0;
}

CHEAT_REGISTER_SERVICE(netvars);
