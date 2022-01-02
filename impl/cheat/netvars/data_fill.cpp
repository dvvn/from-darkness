module;
#include "storage_includes.h"

#ifdef CHEAT_NETVARS_RESOLVE_TYPE
#include <nstd/type name.h>
#endif

#include <nstd/runtime_assert.h>
#include <nstd/format.h>

#include <optional>
#include <algorithm>

module cheat.netvars:data_fill;

using namespace cheat;
using namespace csgo;

namespace cheat::csgo
{
	class QAngle;
	class Vector;
	class Color;
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
	return std::ranges::all_of(name, [](char c)
							   {
								   return c != '.';
							   });
}

bool netvars_impl::add_netvar_to_storage(netvars_storage& root_storage, const std::string_view& name, int offset, [[maybe_unused]] string_or_view_holder&& type)
{
	using namespace std::string_literals;
	using namespace std::string_view_literals;

	auto [entry, added] = root_storage.emplace(name);
	if (added == false)
	{
#ifdef CHEAT_NETVARS_RESOLVE_TYPE
		const auto view = type.view( );
		if (view != nstd::type_name<void*>( ))
		{
			auto& type_obj = entry->find("type"sv)->get_ref<std::string&>( );
			if (view != type_obj)
				type_obj = (std::move(type).str( ));
		}
#endif
	}
	else
	{
		*entry =
		{
		{"offset"s, offset}
#ifndef CHEAT_NETVARS_RESOLVE_TYPE
	  , {"type"s, std::move(type)}
#endif
		};
	}

	return added;
}


auto netvars_impl::add_child_class_to_storage(netvars_storage& storage, const std::string_view& name) -> add_child_t
{
	if (name[0] == 'C' && name[1] != '_')
	{
		runtime_assert(std::isalnum(name[1]));
		std::string class_name;
		//internal csgo classes looks like C_***
		//same classes in shared code look like C***
		class_name.reserve(name.size( ) + 1);
		class_name += "C_";
		class_name.append(std::next(name.begin( )), name.end( ));
		return storage.emplace(std::move(class_name));
	}
	else
	{
		runtime_assert(!name.starts_with("DT_"));
		return storage.emplace(name);
	}
}

//-----

static bool _Prop_is_length_proxy(const RecvProp& prop)
{
	if (prop.m_ArrayLengthProxy)
		return true;
	const auto lstr = netvars_impl::str_to_lower(prop.m_pVarName);
	return lstr.find("length") != lstr.npos && lstr.find("proxy") != lstr.npos;
};

template <size_t ...I>
static bool _Strcmp_legacy_impl(const char* l, const char* r, std::index_sequence<I...>)
{
	return ((l[I] == r[I]) && ...);
}

template <bool Whole, size_t Size>
static bool _Strcmp_legacy(const char* str, const char(&check)[Size])
{
	return _Strcmp_legacy_impl(str, check, std::make_index_sequence<Whole ? Size : Size - 1>( ));
}

static bool _Prop_is_base_class(const RecvProp& prop)
{
	return _Strcmp_legacy<true>(prop.m_pVarName, "baseclass");
};

static bool _Table_is_array(const RecvTable& table)
{
	return /*!table.props.empty( ) &&*/ std::isdigit(table.props.back( ).m_pVarName[0]);
};

static bool _Table_is_data_table(const RecvTable& table)
{
	//DT_*****
	return _Strcmp_legacy<false>(table.m_pNetTableName, "DT_");
};

void netvars_impl::store_recv_props(netvars_storage& root_tree, netvars_storage& tree, const RecvTable* recv_table, int offset)
{
	// ReSharper disable once CppTooWideScopeInitStatement
	const auto props = [&]
	{
		const auto& raw_props = recv_table->props;

		auto front = raw_props.begin( );
		auto back = std::prev(raw_props.end( ));

		if (_Prop_is_base_class(*front))
			++front;
		if (_Prop_is_length_proxy(*back))
			--back;

		return std::span(front, std::next(back));
	}();

	for (auto itr = props.begin( ); itr != props.end( ); ++itr)
	{
		// ReSharper disable once CppUseStructuredBinding
		const auto& prop = *itr;
		runtime_assert(prop.m_pVarName != nullptr);
		const std::string_view prop_name = prop.m_pVarName;

		if (!_Save_netvar_allowed(prop_name))
			continue;

		const auto real_prop_offset = offset + prop.m_Offset;

		if (prop_name.rfind(']') != prop_name.npos)
		{
			if (prop_name.ends_with("[0]"))
			{
				const std::string_view real_prop_name = {prop_name.begin( ), std::prev(prop_name.end( ), 3)};
				runtime_assert(!real_prop_name.ends_with(']'));
				std::optional array_size = 1u;

				// ReSharper disable once CppUseStructuredBinding
				for (const auto& p : std::span(std::next(itr), props.end( )))
				{
					if (const std::string_view name = (p.m_pVarName); name.starts_with(real_prop_name))
					{
						if (p.m_RecvType == prop.m_RecvType && name.size( ) != real_prop_name.size( ))
						{
							++* array_size;
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
					string_or_view_holder netvar_type;
#ifdef CHEAT_NETVARS_RESOLVE_TYPE
					if (*array_size == 3 && (std::isupper(real_prop_name[5]) && real_prop_name.starts_with("m_")))
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
						netvar_type = type_std_array(type.view( ), *array_size);
					}
#endif
					add_netvar_to_storage(tree, real_prop_name, real_prop_offset, std::move(netvar_type));
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
		runtime_assert(!_Prop_is_length_proxy(prop));
		runtime_assert(!std::isdigit(prop_name[0]));
#endif

		if (prop.m_RecvType != DPT_DataTable)
		{
			string_or_view_holder netvar_type;
#ifdef CHEAT_NETVARS_RESOLVE_TYPE
			netvar_type = type_recv_prop(prop);
#endif
			add_netvar_to_storage(tree, prop_name, real_prop_offset, std::move(netvar_type));
		}
		else
		{
			const auto child_table = prop.m_pDataTable;
			if (!child_table)
				continue;
			const auto& child_props = child_table->props;
			if (child_props.empty( ))
				continue;

			if (_Table_is_data_table(*child_table))
			{
				store_recv_props(root_tree, tree, child_table, real_prop_offset);
			}
			else if (_Table_is_array(*child_table))
			{
				auto array_begin = child_props.begin( );
				if (_Prop_is_length_proxy(*array_begin))
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
				string_or_view_holder netvar_type;
#endif
				if (array_begin->m_RecvType != DPT_DataTable)
				{
#ifdef CHEAT_NETVARS_RESOLVE_TYPE
					netvar_type = type_recv_prop(*array_begin);
#endif
				}
				else
				{
					const std::string_view child_table_name = child_table->m_pNetTableName;
					string_or_view_holder child_table_unique_name;
					if (prop_name != child_table_name)
						child_table_unique_name = child_table_name;
					else
						child_table_unique_name = std::format("{}_t", child_table_name);

					auto [new_tree, added] = add_child_class_to_storage(root_tree, child_table_unique_name.view( ));
					if (!added)
						continue;
#ifdef CHEAT_NETVARS_RESOLVE_TYPE
					netvar_type = std::move(child_table_unique_name);
#endif
					store_recv_props(root_tree, (*new_tree), array_begin->m_pDataTable, /*real_prop_offset*/0);
				}

				string_or_view_holder netvar_type_array;
#ifdef CHEAT_NETVARS_RESOLVE_TYPE
				const auto array_size = std::distance(array_begin, child_props.end( ));
				netvar_type_array = type_std_array(netvar_type.view( ), array_size);
#endif
				add_netvar_to_storage(tree, prop_name, real_prop_offset, std::move(netvar_type_array));
			}
			else
			{
				runtime_assert("Unknown netvar type");
			}
		}
	}
}

void netvars_impl::iterate_client_class(netvars_storage& root_tree, ClientClass* root_class)
{
	for (auto client_class = root_class; client_class != nullptr; client_class = client_class->pNext)
	{
		const auto recv_table = client_class->pRecvTable;
		if (!recv_table || recv_table->props.empty( ))
			continue;

		auto [new_tree, added] = add_child_class_to_storage(root_tree, client_class->pNetworkName);
		runtime_assert(added == true);

		store_recv_props(root_tree, (*new_tree), recv_table, 0);

		if ((*new_tree).empty( ))
			root_tree.erase(new_tree);
	}
}

void netvars_impl::store_datamap_props(netvars_storage& tree, datamap_t* map)
{
	// ReSharper disable once CppUseStructuredBinding
	for (const auto& desc : map->data)
	{
		if (desc.fieldType == FIELD_EMBEDDED)
		{
			if (desc.TypeDescription != nullptr)
				runtime_assert("Embedded datamap detected");
		}
		else if (desc.fieldName != nullptr)
		{
			const std::string_view field_name = (desc.fieldName);

			if (!_Save_netvar_allowed(field_name))
				continue;

			const auto offset = desc.fieldOffset[TD_OFFSET_NORMAL];

			string_or_view_holder field_type;
#ifdef CHEAT_NETVARS_RESOLVE_TYPE
			field_type = type_datamap_field(desc);
#endif
			add_netvar_to_storage(tree, field_name, offset, std::move(field_type));
		}
	}
}

void netvars_impl::iterate_datamap(netvars_storage& root_tree, datamap_t* root_map)
{
	for (auto map = root_map; map != nullptr; map = map->baseMap)
	{
		if (map->data.empty( ))
			continue;

		auto [tree, added] = add_child_class_to_storage(root_tree, map->dataClassName);

		// ReSharper disable once CppRedundantCastExpression
		store_datamap_props((*tree), map);

		if (added && (*tree).empty( ))
			root_tree.erase(tree);
	}
}
