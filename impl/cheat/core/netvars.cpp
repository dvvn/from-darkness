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

static void _Save_netvar(property_tree::ptree& storage, string&& name, int offset)
{
	if (!storage.get_child_optional(name))
		storage.put(move(name), offset);
}

static void _Store_recv_props(property_tree::ptree& storage, RecvTable* recv_table, int offset)
{
	static constexpr auto baseclass = string_view("baseclass");

	// ReSharper disable once CppUseStructuredBinding
	for (auto& prop: span(recv_table->m_pProps, recv_table->m_nProps))
	{
		BOOST_ASSERT_MSG(!isdigit(prop.m_pVarName[0]), "Netvar dumper inside array datable!");

		const auto prop_name = string_view(prop.m_pVarName);
		if (prop_name == baseclass)
			continue;

		const auto real_prop_offset = offset + prop.m_Offset;

		if (prop.m_RecvType != DPT_DataTable)
		{
			_Save_netvar(storage, string(prop_name), real_prop_offset);
		}
		else
		{
			//rewrite this
			//class -> class ; not class -> offset + offset + (class)offset
#if 0
			
			const auto child_table = prop.m_pDataTable;

			if (!child_table)
				continue;
			if (child_table->m_nProps == 0)
				continue;

			//("DT" - "DataTable")
			const auto net_table_name = child_table->m_pNetTableName;

			if (net_table_name[0] == 'D' && net_table_name[1] == 'T')
			{
				_Store_recv_props(storage, child_table, real_prop_offset);
			}
			else
			{
				//todo: mark it as array
				_Save_netvar(storage, prop_name, {real_prop_offset, depth, (&prop)});
			}
#endif
		}
	}
}

static string _Fix_class_name(string_view name)
{
	string class_name;
	if (name[0] == 'C' && name[1] != '_')
	{
		//internal csgo classes looks like C_***
		//same classes in shared code look like C***
		class_name.reserve(name.size( ) + 1);
		class_name += "C_";
		class_name.append(std::next(name.begin( )), name.end( ));
	}
	else
	{
		class_name = name;
	}
	return class_name;
}

static void _Iterate_client_class(property_tree::ptree& storage, ClientClass* root_class)
{
	for (auto client_class = root_class; client_class != nullptr; client_class = client_class->pNext)
	{
		if (client_class->pRecvTable == nullptr)
			continue;

		auto class_name = _Fix_class_name(client_class->pNetworkName);
		BOOST_ASSERT(!storage.get_child_optional(class_name));

		auto& child = storage.add_child(move(class_name), { });
		_Store_recv_props(child, client_class->pRecvTable, 0);
	}
}

static void _Store_datamap_props(property_tree::ptree& storage, datamap_t* map)
{
	// ReSharper disable once CppUseStructuredBinding
	for (auto& desc: span(map->dataDesc, map->dataNumFields))
	{
		BOOST_ASSERT(desc.fieldName != nullptr);

		if (desc.fieldType != FIELD_EMBEDDED)
		{
			const auto offset = desc.fieldOffset[TD_OFFSET_NORMAL];
			_Save_netvar(storage, desc.fieldName, offset);
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

static void _Iterate_datamap(property_tree::ptree& storage, datamap_t* root_map)
{
	for (auto map = root_map; map != nullptr; map = map->baseMap)
	{
		auto class_name = string(map->dataClassName);

		if (auto dumps = storage.get_child_optional(class_name); dumps.has_value( ))
			_Store_datamap_props(*dumps, map);
		else
			_Store_datamap_props(storage.add_child(move(class_name), { }), map);
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
#ifdef _DEBUG //dump first time in release mode, debug mode ultra slow!
	dumps_path = filesystem::path(CHEAT_DUMPS_FOLDER) / client_dll->name_wide( ) / to_wstring(client_dll->check_sum( )) / L"vtables.json";
#else
	(void)dumps_path;
#endif
	const auto load_result = vtables.load(dumps_path);
	(void)load_result;
	BOOST_ASSERT(load_result == mem::data_cache_result::success);

	const auto baseent = vtables.get_cache( ).at("C_BaseEntity").addr.raw<C_BaseEntity>( );

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

int netvars::at(const utl::string_view& path) const
{
	return data__.get<int>(string(path));
}
