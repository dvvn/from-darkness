#include "netvars.h"

#include "console.h"
#include "csgo interfaces.h"

#include "cheat/sdk/IBaseClientDll.hpp"
#include "cheat/sdk/entity/C_BaseEntity.h"

using namespace cheat;
using namespace csgo;
using namespace detail::netvars;
using namespace utl;

netvar_source::netvar_source(RecvProp* ptr): obj__(*ptr)
{
}

netvar_source::netvar_source(typedescription_t* ptr): obj__(*ptr)
{
}

void* netvar_source::get( ) const
{
	return (visit(overload([](const reference_wrapper<typedescription_t>& ptr)-> void*
						   {
							   return addressof(ptr.get( ));
						   }, [](const reference_wrapper<RecvProp>& ptr)-> void*
						   {
							   return addressof(ptr.get( ));
						   }), obj__));
}

const char* netvar_source::name( ) const
{
	return visit(overload([](const reference_wrapper<typedescription_t>& ptr)-> const char*
						  {
							  return ptr.get( ).fieldName;
						  }, [](const reference_wrapper<RecvProp>& ptr)-> const char*
						  {
							  return ptr.get( ).m_pVarName;
						  }), obj__);
}

const netvar_prop* dumped_class::find(const string_view& name) const
{
#if 0
	static_assert(ranges::random_access_range<decltype(props__)>);

	for(auto& p : props__)
	{
		if(p.name == name)
			return addressof(p);
	}

	return nullptr;

#endif
	const auto found = props__.find((name));
	return found == props__.end( ) ? nullptr : addressof(found->second);
}

pair<netvar_prop&, bool> dumped_class::try_emplace(const string_view& name, netvar_prop&& prop)
{
	auto&& [name_itr, placed] = props__.emplace(name, prop);
	return pair<netvar_prop&, bool>(name_itr.value( ), placed);
}

const netvar_prop& dumped_class::at(const string_view& name) const
{
	return props__.at(name);
}

netvars::~netvars( )
{
}

netvars::netvars( )
{
	this->Wait_for<csgo_interfaces>( );
}

static void _Save_netvar(dumped_class& storage, const string_view& name, netvar_prop&& prop)
{
	auto&& [prop_stored, placed] = storage.try_emplace(name, move(prop));

#ifdef _DEBUG
	if (placed)
		return;
	if (prop_stored.offset == prop.offset)
		return;
	if (prop_stored.source.name( ) == prop.source.name( )) //if name pointers same this is same netvar on different offset
		return;

	BOOST_ASSERT("Duplicated netvar detected!");
#endif
}

static void _Store_recv_props(dumped_class& storage, RecvTable* recv_table, netvar_prop::offset_type offset, int depth)
{
	static constexpr auto baseclass = string_view("baseclass");
	static_assert(std::is_same_v<decltype(RecvProp::m_Offset), netvar_prop::offset_type>);

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
			_Save_netvar(storage, prop_name, {real_prop_offset, depth, (&prop)});
		}
		else
		{
			const auto child_table = prop.m_pDataTable;

			if (!child_table)
				continue;
			if (child_table->m_nProps == 0)
				continue;

			//("DT" - "DataTable")
			const auto net_table_name = child_table->m_pNetTableName;

			if (net_table_name[0] == 'D' && net_table_name[1] == 'T')
			{
				_Store_recv_props(storage, child_table, real_prop_offset, depth + 1);
			}
			else
			{
				//todo: mark it as array
				_Save_netvar(storage, prop_name, {real_prop_offset, depth, (&prop)});
			}
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

static void _Iterate_client_class(classes_storage& storage, ClientClass* root_class)
{
	for (auto client_class = root_class; client_class != nullptr; client_class = client_class->pNext)
	{
		if (client_class->pRecvTable == nullptr)
			continue;

		auto   class_name = _Fix_class_name(client_class->pNetworkName);
		auto&& [props_itr, placed] = storage.try_emplace(move(class_name));

		BOOST_ASSERT(placed == true);

		_Store_recv_props(props_itr.value( ), client_class->pRecvTable, 0, 0);
	}
}

static void _Store_datamap_props(dumped_class& storage, datamap_t* map)
{
	// ReSharper disable once CppUseStructuredBinding
	for (auto& desc: span(map->dataDesc, map->dataNumFields))
	{
		BOOST_ASSERT(desc.fieldName != nullptr);

		if (desc.fieldType != FIELD_EMBEDDED)
		{
			const auto offset = desc.fieldOffset[TD_OFFSET_NORMAL];
			_Save_netvar(storage, desc.fieldName, {offset, 0, &desc});
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

static void _Iterate_datamap(classes_storage& storage, datamap_t* root_map)
{
	for (auto map = root_map; map != nullptr; map = map->baseMap)
	{
		auto class_name = string_view(map->dataClassName);

		if (auto dumps = storage.find(class_name); dumps != storage.end( ))
			_Store_datamap_props(dumps.value( ), map);
		else
			_Store_datamap_props(storage[string(class_name)], map);
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
#ifdef CHEAT_DUMPS_FOLDER
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

const netvar_prop& netvars::at(const string_view& class_name, const string_view& prop_name) const
{
#if 0
	for(auto& d : data__)
	{
		if(d.name( ) != class_name)
			continue;

		const auto found_prop = d.find(prop_name);
		BOOST_ASSERT(found_prop != nullptr);
		return *found_prop;
	}

	BOOST_ASSERT("Class not found");
	return *static_cast<netvar_prop*>(nullptr);

#endif

	return data__.at(class_name).at(prop_name);
}
