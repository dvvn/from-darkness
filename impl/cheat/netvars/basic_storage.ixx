module;

#include <cheat/netvars/basic_storage.h>

#include <vector>
#include <string>
#include <variant>
#include <memory>

export module cheat.netvars.core:basic_storage;
export import cheat.csgo.structs.Recv;
export import cheat.csgo.structs.DataMap;
import cheat.tools.object_name;
import nstd.text.hashed_string;
import nstd.text.string_or_view;

using netvar_info_source = std::variant<cheat::csgo::RecvProp*, cheat::csgo::typedescription_t*>;
using nstd::text::string_or_view;

class basic_netvar_info
{
public:
	virtual ~basic_netvar_info( ) = default;

	virtual size_t offset( ) const  noexcept = 0;
	virtual nstd::hashed_string_view name( ) const noexcept = 0;
	virtual std::string_view type( ) const noexcept = 0;
};

class netvar_info final :public basic_netvar_info
{
	size_t offset_;
	netvar_info_source source_;
	size_t size_;//for arrays
	mutable nstd::hashed_string_view name_;
	mutable string_or_view type_;

public:
	netvar_info(const size_t offset, const netvar_info_source source, const size_t size = 0, const nstd::hashed_string_view name = {});

	size_t offset( ) const noexcept;
	nstd::hashed_string_view name( ) const noexcept;
	std::string_view type( ) const noexcept;
};

template<typename Fn>
class netvar_info_custom final :public basic_netvar_info
{
	mutable std::variant<size_t, Fn> getter_;
	nstd::hashed_string_view name_;
	string_or_view type_;

public:
	netvar_info_custom(Fn&& getter, const nstd::hashed_string_view name = {}, string_or_view&& type = {})
		:getter_(std::move(getter)), name_(name), type_(type)
	{
	}

	size_t offset( ) const noexcept
	{
		if(std::holds_alternative<size_t>(getter_))
			return std::get<0>(getter_);

		const auto offset = std::invoke(std::get<1>(getter_));
		getter_ = offset;
		return offset;
	}

	nstd::hashed_string_view name( ) const noexcept
	{
		return name_;
	}

	std::string_view type( ) const noexcept
	{
		return type_;
	}
};

class netvar_info_custom_constant final :public basic_netvar_info
{
	size_t offset_;
	nstd::hashed_string_view name_;
	string_or_view type_;

public:
	netvar_info_custom_constant(const size_t offset, const nstd::hashed_string_view name = {}, string_or_view&& type = {});

	size_t offset( ) const noexcept;
	nstd::hashed_string_view name( ) const noexcept;
	std::string_view type( ) const noexcept;
};

template<typename T, class Base = std::vector<T>>
struct private_vector :protected Base
{
	using Base::Base;

	using Base::begin;
	using Base::end;
	using Base::empty;
	using Base::size;

	using Base::value_type;
};

class netvar_table : public private_vector<std::unique_ptr<basic_netvar_info>>
{
	void validate_item(const basic_netvar_info* info) const noexcept;

	template<typename T, typename ...Args>
	const T* add_impl(Args&&...args) noexcept
	{
		auto uptr = std::make_unique<T>(std::forward<Args>(args)...);
		const T* ret = uptr.get( );
		this->emplace_back(std::move(uptr));
		this->validate_item(ret);
		return ret;
	}

	nstd::hashed_string name_;

public:
	netvar_table(nstd::hashed_string&& name);

	nstd::hashed_string_view name( ) const noexcept;

	const basic_netvar_info* find(const nstd::hashed_string_view name) const noexcept;

	const netvar_info* add(const size_t offset, const netvar_info_source source, const size_t size = 0, const nstd::hashed_string_view name = {}) noexcept;
	const netvar_info_custom_constant* add(const size_t offset, const nstd::hashed_string_view name, string_or_view&& type = {}) noexcept;
	template<std::invocable Fn>
	auto add(Fn&& getter, const nstd::hashed_string_view name, string_or_view&& type = {}) noexcept
	{
		return add_impl<netvar_info_custom<std::remove_cvref_t<Fn>>>(std::forward<Fn>(getter), name, std::move(type));
	}
	template<typename Type, typename TypeProj = std::identity, typename From>
	auto add(From&& from, const nstd::hashed_string_view name, TypeProj proj = {}) noexcept
	{
		return add(std::forward<From>(from), name, std::invoke(proj, cheat::tools::csgo_object_name<Type>( )));
	}
};

export namespace cheat::netvars
{
	class basic_storage :public private_vector<netvar_table>
	{
	public:
		//[[deprecated]]
		//bool contains_duplicate(const nstd::hashed_string_view name, netvar_table* const from = nullptr) const noexcept;

		const netvar_table* find(const nstd::hashed_string_view name) const noexcept;
		netvar_table* find(const nstd::hashed_string_view name) noexcept;
		template<typename T>
		auto find( ) noexcept
		{
			return this->find(tools::csgo_object_name<T>( ));
		}

		netvar_table* add(netvar_table&& table, const bool skip_find = false) noexcept;
		netvar_table* add(nstd::hashed_string&& name, const bool skip_find = false) noexcept;
	};
}