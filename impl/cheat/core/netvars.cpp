#include "netvars.h"

#include "console.h"
#include "csgo interfaces.h"

#include "cheat/sdk/IBaseClientDll.hpp"
#include "cheat/sdk/entity/C_BaseEntity.h"

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

template <typename Nstr, typename Tstr>
static bool _Save_netvar(property_tree::ptree& storage, Nstr&& name, int offset, Tstr&& type, size_t repeat)
{
	const auto path = property_tree::ptree::path_type(string(forward<Nstr>(name)));
	if (storage.get_child_optional(path))
		return false;
	auto& entry = storage.add_child(path, { });
	entry.put(_STRINGIZE(offset), offset);
#ifdef _DEBUG
	entry.put(_STRINGIZE(type), string(forward<Tstr>(type)));
	BOOST_ASSERT(repeat > 0);
	entry.put(_STRINGIZE(repeat), repeat);
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
		BOOST_ASSERT(!class_name.starts_with("DT"));
	}

	property_tree::ptree* tree;

	const auto child = storage.get_child_optional(class_name);
	if (!child)
		tree = addressof(storage.add_child(move(class_name), { }));
	else
		tree = child.get_ptr( );

	return {tree, !child};
}

static string _Str_to_lower(const string_view& str)
{
	string ret;
	ret.reserve(str.size( ));
	for (auto& c: str)
		ret += std::tolower(c);
	return ret;
}

static void _Store_recv_props(property_tree::ptree& root_tree, property_tree::ptree& tree, RecvTable* recv_table, int offset)
{
	const auto& props = recv_table->props;

	for (auto i = props[0].m_pVarName == string_view("baseclass") ? 1 : 0; i < props.size( ); ++i)
	{
		// ReSharper disable once CppUseStructuredBinding
		const auto& prop = props[i];
		auto        prop_name = string_view(prop.m_pVarName);

		if (static const auto path_default_separator = property_tree::ptree::path_type( ).separator( );
			prop_name.find(path_default_separator) != prop_name.npos)
		{
			continue;
		}
		else if (path_default_separator != '.' && prop_name.rfind('.') != prop_name.npos)
		{
			continue;
		}
		if (prop_name.rfind(']') != prop_name.npos)
		{
			continue;
		}

		size_t array_repeat;
		if (!isdigit(prop_name[0]))
		{
			array_repeat = 1;
		}
		else
		{
			BOOST_ASSERT(prop_name[0] == '0');

			const auto part = props.subspan(i + 1);
			const auto array_end_itr = ranges::find_if_not(part, isdigit, [](const RecvProp& rp) { return rp.m_pVarName[0]; });
			const auto array_end_num = std::distance(part.begin( ), array_end_itr);

			array_repeat = array_end_num - i + 1;
			i += array_end_num;

			static const string props_array = "m_PropsArray";
			prop_name = props_array;
		}

		/*if (prop.m_ArrayLengthProxy != nullptr)
		{
			continue;
		}*/

		const auto real_prop_offset = offset + prop.m_Offset;

		if (prop.m_RecvType != DPT_DataTable)
		{
			_Save_netvar(tree, prop_name, real_prop_offset, "simple type", array_repeat);
		}
		else
		{
			const auto child_table = prop.m_pDataTable;
			if (!child_table || child_table->props.empty( ))
				continue;

			const auto child_table_name = string_view(child_table->m_pNetTableName);
			const auto child_table_is_proxy = [&]
			{
				return _Str_to_lower(child_table_name).find("proxy") != string::npos;
			};
			const auto child_table_is_data_table = [&]
			{
				return child_table_name.starts_with("DT");
			};

			if (child_table_is_data_table( ) || child_table_is_proxy( ))
			{
				_Store_recv_props(root_tree, tree, child_table, real_prop_offset);
			}
			else
			{
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

				if (auto [new_tree, added] = _Add_child_class(root_tree, child_table_unique_name);
					added == true)
				{
					_Save_netvar(tree, prop_name, real_prop_offset, child_table_unique_name, array_repeat);
					_Store_recv_props(root_tree, *new_tree, child_table, real_prop_offset);
				}
			}

			(void)child_table_is_data_table;
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
		BOOST_ASSERT(added==true);

		_Store_recv_props(root_tree, *new_tree, recv_table, 0);
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
			_Save_netvar(tree, desc.fieldName, offset, "simple type datamap", 1);
		}
		else
		{
			if (desc.TypeDescription != nullptr)
			{
				BOOST_ASSERT("Internal datamap detected");
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
	}
}

void netvars::Load( )
{
#ifdef CHEAT_GUI_TEST

#else

	_Iterate_client_class(data__, csgo_interfaces::get( ).client->GetAllClasses( ));

	const auto client_dll = mem::all_modules::get( ).find("client.dll");
	auto&      vtables = client_dll->vtables( );

	filesystem::path dumps_path;
#if defined(_DEBUG) /*|| 1*/ //dump first time in release mode, debug mode ultra slow!
	dumps_path = filesystem::path(CHEAT_DUMPS_FOLDER) / client_dll->name_wide( ) / to_wstring(client_dll->check_sum( )) / L"vtables.json";
#else
	(void)dumps_path;
#endif
	const auto load_result = vtables.load(dumps_path);
	(void)load_result;
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
	return data__.get<int>(string(path));
}
