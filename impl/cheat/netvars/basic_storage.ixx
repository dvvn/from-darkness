module;

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
import nstd.indexed_iterator;

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
public:
	netvar_info(const size_t offset, const netvar_info_source source, const size_t size = 0, const nstd::hashed_string_view name = {});

	size_t offset( ) const noexcept;
	nstd::hashed_string_view name( ) const noexcept;
	std::string_view type( ) const noexcept;

private:
	size_t offset_;
	netvar_info_source source_;
	size_t size_;//for arrays
	mutable nstd::hashed_string_view name_;
	mutable string_or_view type_;
};

template<typename Fn>
class netvar_info_custom final :public basic_netvar_info
{
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

private:
	mutable std::variant<size_t, Fn> getter_;
	nstd::hashed_string_view name_;
	string_or_view type_;
};

class netvar_info_custom_constant final :public basic_netvar_info
{
public:
	netvar_info_custom_constant(const size_t offset, const nstd::hashed_string_view name = {}, string_or_view&& type = {});

	size_t offset( ) const noexcept;
	nstd::hashed_string_view name( ) const noexcept;
	std::string_view type( ) const noexcept;

private:
	size_t offset_;
	nstd::hashed_string_view name_;
	string_or_view type_;
};

class netvar_table
{
	void validate_item(const basic_netvar_info* info) const noexcept;

	template<typename T, typename ...Args>
	const T* add_impl(Args&&...args) noexcept
	{
		auto uptr = std::make_unique<T>(std::forward<Args>(args)...);
		const T* ret = uptr.get( );
		storage_.emplace_back(std::move(uptr));
		this->validate_item(ret);
		return ret;
	}

public:
	using element_type = std::unique_ptr<basic_netvar_info>;
	using storage_type = std::vector<element_type>;
	using iterator = nstd::indexed_iterator<const storage_type>;

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

	iterator begin( ) const noexcept;
	iterator end( ) const noexcept;
	bool empty( ) const noexcept;

private:
	nstd::hashed_string name_;
	storage_type storage_;
};

export namespace cheat::netvars
{
	class basic_storage
	{
	public:
		using storage_type = std::vector<netvar_table>;
		using iterator = nstd::indexed_iterator<storage_type>;
		using const_iterator = nstd::indexed_iterator<const storage_type>;

		//[[deprecated]]
		//bool contains_duplicate(const nstd::hashed_string_view name, netvar_table* const from = nullptr) const noexcept;

		const_iterator find(const nstd::hashed_string_view name) const noexcept;
		iterator find(const nstd::hashed_string_view name) noexcept;
		template<typename T>
		auto find( ) noexcept
		{
			return this->find(tools::csgo_object_name<T>( ));
		}

		iterator add(netvar_table&& table, const bool skip_find = false) noexcept;
		iterator add(nstd::hashed_string&& name, const bool skip_find = false) noexcept;

		const_iterator begin( ) const noexcept;
		const_iterator end( ) const noexcept;
		bool empty( ) const noexcept;
		size_t size( ) const noexcept;

	private:
		storage_type storage_;
	};
}