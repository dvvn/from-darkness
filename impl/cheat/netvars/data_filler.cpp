#include "data_filler.h"
#include "config.h"

#include <nstd/runtime assert.h>
#include <nstd/type name.h>

using namespace cheat::detail;
using namespace cheat::csgo;

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
	return !std::ranges::any_of(name, [](char c)
	{
		return c == '.';
	});
}

bool cheat::detail::_Save_netvar(netvars_storage& storage,const std::string_view& name, int offset, netvar_type_holder&& type)
{

	auto&& [entry, added] = storage.emplace(std::string(name), netvars_storage{});
	if (added == false)
	{
#ifdef CHEAT_NETVARS_RESOLVE_TYPE
		if (type != nstd::type_name<void*>)
		{
			auto& type_obj = entry->at("type").get_ref<std::string&>( );
			if (type_obj != type)
				type_obj.assign(static_cast<std::string&&>(type));
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

std::pair<netvars_storage::iterator, bool> cheat::detail::_Add_child_class(netvars_storage& storage, const std::string_view& name)
{
	std::string class_name;
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
		class_name = (name);
		runtime_assert(!class_name.starts_with("DT_"));
	}

	return storage.emplace(std::move(class_name), netvars_storage::value_type{});
}

void  cheat::detail::_Store_recv_props(netvars_storage& root_tree, netvars_storage& tree, const RecvTable* recv_table, int offset)
{
	static constexpr auto prop_is_length_proxy = [](const RecvProp& prop)
	{
		if (prop.m_ArrayLengthProxy)
			return true;
		const auto lstr = str_to_lower(prop.m_pVarName);
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
					auto [new_tree, added] = _Add_child_class(root_tree,(child_table_unique_name));
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

void cheat::detail::_Iterate_client_class(netvars_storage& root_tree, csgo::ClientClass* root_class)
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

void cheat::detail::_Store_datamap_props(netvars_storage& tree, csgo::datamap_t* map)
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

void cheat::detail::_Iterate_datamap(netvars_storage& root_tree, csgo::datamap_t* root_map)
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
