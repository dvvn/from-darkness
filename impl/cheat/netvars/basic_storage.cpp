module;

#include <nstd/overload.h>
#include <nstd/runtime_assert.h>
#include <nstd/ranges.h>

#include <vector>
#include <string>
#include <variant>
#include <functional>

module cheat.netvars.core:basic_storage;
import :type_resolve;

using namespace cheat;
using namespace netvars;
using namespace csgo;

static const char* _Netvar_name(const netvar_info_source source) noexcept
{
	return std::visit(nstd::overload(&RecvProp::m_pVarName, &typedescription_t::fieldName), source);
}

template</*typename Ret,*/ typename T, typename Fn, typename ...Funcs>
static decltype(auto) /*Ret*/ _Select_invoke(T arg, Fn fn, Funcs ...funcs) noexcept
{
	if constexpr (std::invocable<Fn, T>)
		return std::invoke(fn, arg);
	else
		return _Select_invoke(arg, funcs...);
}

static string_or_view _Netvar_type(const netvar_info_source source) noexcept
{
	return std::visit([]<class T>(T* const ptr) noexcept -> string_or_view
	{
		return _Select_invoke(ptr, type_recv_prop, type_datamap_field);
	}, source);
}

template<typename T>
static auto _Find_name(T&& data, const nstd::hashed_string_view name) noexcept -> decltype(get_ptr(data[0]))
{
	for (auto& entry : data)
	{
		auto ptr = get_ptr(entry);
		if (ptr->name( ) == name)
			return ptr;
	}
	return nullptr;
}

template<typename T>
static safe_iterator<T> _Find_name_safe(T& data, const nstd::hashed_string_view name) noexcept
{
	for (size_t i = 0; i < data.size( ); ++i)
	{
		if (get_ref(data[i]).name( ) == name)
			return {data, i};
	}
	return {};
}

//---

netvar_info::netvar_info(const size_t offset, const netvar_info_source source, const size_t size, const nstd::hashed_string_view name)
	:offset_(offset), source_(source), size_(size), name_(name)
{
}

size_t netvar_info::offset( ) const noexcept
{
	return offset_;
}

//const void* netvar_info::source( ) const
//{
//	const auto addr = reinterpret_cast<uintptr_t>(std::addressof(type_));
//	const auto inner_ptr = reinterpret_cast<void**>(addr);
//	return *inner_ptr;
//}

nstd::hashed_string_view netvar_info::name( ) const noexcept
{
	if (name_.empty( ))
	{
		if (size_ == 0)
		{
			name_ = _Netvar_name(source_);
		}
		else
		{
			std::string_view name = _Netvar_name(source_);
			runtime_assert(name.ends_with("[0]"));
			name.remove_suffix(3);
			runtime_assert(name.rfind(']') == name.npos);
			name_ = name;
		}
	}
	return name_;
}

std::string_view netvar_info::type( ) const noexcept
{
	const std::string_view type_strv = type_;
	if (!type_strv.empty( ))
		return type_strv;

	auto type = _Netvar_type(source_);

	if (size_ <= 1)
	{
		type_ = std::move(type);
	}
	else
	{
		std::string_view netvar_type;
		if (size_ == 3)
		{
			netvar_type = std::visit([&]<class T>(T* const ptr)
			{
				return type_array_prefix(type, ptr);
			}, source_);
		}

		if (netvar_type.empty( ))
			type_ = type_std_array(type, size_);
		else
			type_ = netvar_type;

	}

	return type_;
}

//----

netvar_info_custom_constant::netvar_info_custom_constant(const size_t offset, const nstd::hashed_string_view name, string_or_view&& type)
	:offset_(offset), name_(name), type_(type)
{
}

size_t netvar_info_custom_constant::offset( ) const noexcept
{
	return offset_;
}

nstd::hashed_string_view netvar_info_custom_constant::name( ) const noexcept
{
	return name_;
}

std::string_view netvar_info_custom_constant::type( ) const noexcept
{
	return type_;
}

//----

void netvar_table::validate_item(const basic_netvar_info* info) const noexcept
{
	const auto name = info->name( );
	runtime_assert(!name.empty( ), "Item name not set!");

	if (data_.empty( ))
		return;

	const auto offset = info->offset( );
	const auto type = info->type( );

	for (auto& item : data_)
	{
		auto& ref = get_ref(item);
		const auto name0 = ref.name( );
		if (ref.name( ) == name)
			runtime_assert("Item with given name already added!");

		if (ref.offset( ) == offset)
		{
			const auto type0 = ref.type( );
			if (type0.empty( ) || type.empty( ) || type == type0)
				runtime_assert("Item with given offset and type already added!");
			//othervise skip this offset manually
		}

	}
}

netvar_table::netvar_table(nstd::hashed_string&& name)
	:name_(std::move(name))
{
}

nstd::hashed_string_view netvar_table::name( ) const noexcept
{
	return name_;
}

const basic_netvar_info* netvar_table::find(const nstd::hashed_string_view name) const noexcept
{
	return _Find_name(data_, name);
}

const netvar_info* netvar_table::add(const size_t offset, const netvar_info_source source, const size_t size, const nstd::hashed_string_view name) noexcept
{
	return add_impl<netvar_info>(offset, source, size, name);
}

const netvar_info_custom_constant* netvar_table::add(const size_t offset, const nstd::hashed_string_view name, string_or_view&& type) noexcept
{
	return add_impl<netvar_info_custom_constant>(offset, name, std::move(type));
}

bool netvar_table::empty( ) const noexcept
{
	return data_.empty( );
}

auto netvar_table::begin( ) const noexcept -> iterator
{
	return {data_, 0u};
}

auto netvar_table::end( ) const noexcept -> iterator
{
	return {data_, data_.size( )};
}

//----

bool basic_storage::contains_duplicate(const nstd::hashed_string_view name, netvar_table* const from) const noexcept
{
	const auto begin = data_.data( );
	const auto end = begin + data_.size( );

	const auto pos = from ? from : begin;
	runtime_assert(std::distance(begin, pos) >= 0);

	const auto found1 = _Find_name(std::span(pos, end), name);
	if (found1)
		return _Find_name(std::span(found1 + 1, end), name);
	return false;
}

auto basic_storage::find(const nstd::hashed_string_view name) const noexcept -> const_iterator
{
	return _Find_name_safe(data_, name);
}

auto basic_storage::find(const nstd::hashed_string_view name) noexcept -> iterator
{
	return _Find_name_safe(data_, name);
}

auto basic_storage::add(netvar_table&& table, const bool skip_find) noexcept -> iterator
{
	if (!skip_find)
	{
		const auto existing = _Find_name_safe(data_, table.name( ));
		if (existing)
			return existing;
	}
	data_.push_back(std::move(table));
	return {data_,data_.size( ) - 1};
}

auto basic_storage::add(nstd::hashed_string&& name, const bool skip_find) noexcept -> iterator
{
	if (!skip_find)
	{
		const auto existing = _Find_name_safe(data_, name);
		if (existing)
			return existing;
	}
	data_.emplace_back(std::move(name));
	return {data_, data_.size( ) - 1};
}

bool basic_storage::empty( ) const noexcept
{
	return data_.empty( );
}

auto basic_storage::begin( ) const noexcept -> const_iterator
{
	return {data_, 0u};
}

auto basic_storage::end( ) const noexcept -> const_iterator
{
	return {data_, data_.size( )};
}

size_t basic_storage::size( ) const noexcept
{
	return data_.size( );
}