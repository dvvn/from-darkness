module;

#include <nstd/runtime_assert.h>
#include <nstd/format.h>
#include <nstd/ranges.h>

#include <cctype>
#include <algorithm>
#include <variant>

module cheat.netvars.storage;
import nstd.text.convert;

using namespace cheat;
using namespace netvars;
using namespace csgo;

static std::string correct_class_name(const std::string_view name)
{
	std::string ret;

	if (name[0] == 'C' && name[1] != '_')
	{
		runtime_assert(std::isalnum(name[1]));
		//internal csgo classes looks like C_***
		//same classes in shared code look like C***
		ret.reserve(2 + name.size( ) - 1);
		ret += "C_";
		ret += name.substr(1);
	}
	else
	{
		runtime_assert(!name.starts_with("DT_"));
		ret.assign(name);
	}

	return ret;
}

static bool can_skip_netvar(const char* name)
{
	for (;;)
	{
		const auto c = *++name;
		if (c == '.')
			return true;
		if (c == '\0')
			return false;
	}

}

static bool can_skip_netvar(const nstd::text::string_view name)
{
	return name.contains('.');
}

static bool _Table_is_array(const RecvTable& table)
{
	return /*!table.props.empty( ) &&*/ std::isdigit(table.props.back( ).m_pVarName[0]);
};

static bool _Table_is_data_table(const RecvTable& table)
{
	//DT_*****
	return std::memcmp(table.m_pNetTableName, "DT_", 3) == 0 && table.m_pNetTableName[3] != '\0';
};

static std::pair<RecvProp*, RecvProp*> get_props_range(const RecvTable* recv_table)
{
	constexpr auto is_base_class = [](const RecvProp* prop)
	{
		constexpr std::string_view str = "baseclass";
		return std::memcmp(prop->m_pVarName, str.data( ), str.size( )) == 0 && prop->m_pVarName[str.size( )] == '\0';
	};

	constexpr auto is_length_proxy = [](const RecvProp* prop)
	{
		if (prop->m_ArrayLengthProxy)
			return true;

		const auto lstr = nstd::text::to_lower(prop->m_pVarName);
		return lstr.contains("length") && lstr.contains("proxy");

	};

	const auto& raw_props = recv_table->props;

	auto front = raw_props.data( );
	auto back = front + raw_props.size( ) - 1;

	if (is_base_class(front))
		++front;
	if (is_length_proxy(back))
		--back;

	return {front, back + 1};
}

struct recv_prop_array_info
{
	std::string_view name;
	size_t size = 0;
};

//other_props = {itr+1, end}
static recv_prop_array_info parse_prop_array(const std::string_view first_prop_name, const std::span<const RecvProp> other_props, const netvar_table& tree)
{
	if (!first_prop_name.ends_with("[0]"))
		return {};

	const std::string_view real_prop_name = first_prop_name.substr(0, first_prop_name.size( ) - 3);
	runtime_assert(!real_prop_name.ends_with(']'));
	if (tree.find(real_prop_name))//todo: debug break for type check!
		return {real_prop_name,0};

	//todo: try extract size from length proxy
	size_t array_size = 1;

	for (const auto& prop : other_props)
	{
		if (prop.m_RecvType != prop.m_RecvType)//todo: check is name still same after this (because previously we store this name without array braces)
			break;

		//name.starts_with(real_prop_name)
		if (std::memcmp(prop.m_pVarName, real_prop_name.data( ), real_prop_name.size( )) != 0)
			break;

		//name.size() == real_prop_name.size()
		if (prop.m_pVarName[real_prop_name.size( )] != '\0')
			break;

		++array_size;
	}

	/*string_or_view_holder netvar_type;
#ifdef CHEAT_NETVARS_RESOLVE_TYPE
	if (array_size == 3 && (std::isupper(real_prop_name[5]) && real_prop_name.starts_with("m_")))
	{
		auto prefix = real_prop_name.substr(2, 3);
		if (prop.m_RecvType == DPT_Float)
		{
			if (prefix == "ang")
				netvar_type = nstd::type_name<QAngle>( );
			else if (prefix == "vec")
				netvar_type = nstd::type_name<Vector>( );
		}
		else if (prop.m_RecvType == DPT_Int)
		{
			if (prefix == "uch")
				netvar_type = nstd::type_name<Color>( );
		}
	}

	if (netvar_type.view( ).empty( ))
	{
		auto type = type_recv_prop(prop);
		netvar_type = type_std_array(type.view( ), array_size);
	}
#endif
	add_netvar_to_storage(tree, real_prop_name, real_prop_offset, std::move(netvar_type));
	itr += *array_size - 1;*/

	return {real_prop_name,array_size};
}

static void parse_client_class(storage& root_tree, netvar_table& tree, csgo::RecvTable* recv_table, const size_t offset)
{
	const auto [props_begin, props_end] = get_props_range(recv_table);

	for (auto itr = props_begin; itr != props_end; ++itr)
	{
		const auto& prop = *itr;
		runtime_assert(prop.m_pVarName != nullptr);
		const std::string_view prop_name = prop.m_pVarName;
		if (can_skip_netvar(prop_name))
			continue;

		const auto real_prop_offset = offset + prop.m_Offset;

		if (prop_name.rfind(']') != prop_name.npos)
		{
			const auto array_info = parse_prop_array(prop_name, {itr + 1,props_end}, tree);
			if (array_info.size > 0)
			{
				tree.add(real_prop_offset, itr, array_info.size,array_info.name);
				itr += array_info.size - 1;
			}
		}
		else if (prop.m_RecvType != DPT_DataTable)
		{
			tree.add(real_prop_offset, itr, 0, prop_name);
		}
		else if (prop.m_pDataTable && !prop.m_pDataTable->props.empty( ))
		{
			parse_client_class(root_tree, tree, prop.m_pDataTable, real_prop_offset);
		}

		//			if (_Table_is_data_table(*child_table))
		//			{
		//				parse_client_class(root_tree, tree, child_table, real_prop_offset);
		//			}
		//			else if (_Table_is_array(*child_table))
		//			{
		//				auto array_begin = child_props.begin( );
		//				if (_Prop_is_length_proxy(*array_begin))
		//				{
		//					++array_begin;
		//					runtime_assert(array_begin->m_pVarName[0] == '0');
		//				}
		//				runtime_assert(array_begin != child_props.end( ));
		//
		//#ifdef CHEAT_NETVARS_RESOLVE_TYPE
		//				string_or_view_holder netvar_type;
		//#endif
		//				if (array_begin->m_RecvType != DPT_DataTable)
		//				{
		//#ifdef CHEAT_NETVARS_RESOLVE_TYPE
		//					netvar_type = type_recv_prop(*array_begin);
		//#endif
		//				}
		//				else
		//				{
		//					const std::string_view child_table_name = child_table->m_pNetTableName;
		//					string_or_view_holder child_table_unique_name;
		//					if (prop_name != child_table_name)
		//						child_table_unique_name = child_table_name;
		//					else
		//						child_table_unique_name = std::format("{}_t", child_table_name);
		//
		//					auto [new_tree, added] = add_child_class_to_storage(root_tree, child_table_unique_name.view( ));
		//					if (!added)
		//						continue;
		//#ifdef CHEAT_NETVARS_RESOLVE_TYPE
		//					netvar_type = std::move(child_table_unique_name);
		//#endif
		//					store_recv_props(root_tree, (*new_tree), array_begin->m_pDataTable, /*real_prop_offset*/0);
		//				}
		//
		//				string_or_view_holder netvar_type_array;
		//#ifdef CHEAT_NETVARS_RESOLVE_TYPE
		//				const auto array_size = std::distance(array_begin, child_props.end( ));
		//				netvar_type_array = type_std_array(netvar_type.view( ), array_size);
		//#endif
		//				add_netvar_to_storage(tree, prop_name, real_prop_offset, std::move(netvar_type_array));
		//			}
		//			else
		//			{
		//				runtime_assert("Unknown netvar type");
		//			}

	}
}

void storage::iterate_client_class(csgo::ClientClass* root_class)
{
	for (auto client_class = root_class; client_class != nullptr; client_class = client_class->pNext)
	{
		const auto recv_table = client_class->pRecvTable;
		if (!recv_table || recv_table->props.empty( ))
			continue;

		nstd::hashed_string class_name = correct_class_name(client_class->pNetworkName);
		runtime_assert(!this->find(class_name));
		const auto added = this->add(std::move(class_name), true);

		parse_client_class(*this, *added, recv_table, 0);

		/*if (added->empty( ))
			this->erase(added.data( ));*/
	}
}

static void parse_datamap(netvar_table& tree, csgo::datamap_t* map)
{
	for (auto& desc : map->data)
	{
		if (desc.fieldType == FIELD_EMBEDDED)
		{
			if (desc.TypeDescription != nullptr)
				runtime_assert("Embedded datamap detected");
		}
		else if (desc.fieldName != nullptr)
		{
			const std::string_view name = desc.fieldName;
			if (can_skip_netvar(name))
				continue;
			tree.add(static_cast<size_t>(desc.fieldOffset[TD_OFFSET_NORMAL]), std::addressof(desc), 0, name);
			/*string_or_view_holder field_type;
#ifdef CHEAT_NETVARS_RESOLVE_TYPE
			field_type = type_datamap_field(desc);
#endif
			add_netvar_to_storage(tree, field_name, offset, std::move(field_type));*/
		}
	}
}

void storage::iterate_datamap(csgo::datamap_t* root_map)
{
	for (auto map = root_map; map != nullptr; map = map->baseMap)
	{
		if (map->data.empty( ))
			continue;

		nstd::hashed_string class_name = correct_class_name(map->dataClassName);
		const auto added = this->add(std::move(class_name));

		parse_datamap(*added, map);

		/*if (added->empty( ))
			this->erase(added.data( ));*/
	}
}