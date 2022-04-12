module;

#include <vector>
#include <string>
#include <variant>
#include <memory>

export module cheat.netvars:basic_storage;
export import cheat.csgo.structs.Recv;
export import cheat.csgo.structs.DataMap;
import cheat.tools.object_name;
import nstd.text.hashed_string;
import nstd.text.string_or_view;

struct ref_getter
{
	template<typename T>
	T& operator()(const std::unique_ptr<T>& ptr) const noexcept
	{
		return *ptr;
	}

	template<typename T>
	T& operator()(T& ref) const noexcept
	{
		return ref;
	}
};

template<typename T>
decltype(auto) get_ref(T& ref) noexcept
{
	constexpr ref_getter getter;
	return std::invoke(getter, ref);
}

template<typename T>
auto get_ptr(T& ref) noexcept
{
	return std::addressof(get_ref(ref));
}

template<typename T>
class safe_iterator
{
public:
	safe_iterator( ) = default;

	safe_iterator(T& s, size_t off)
		:source_(std::addressof(s)), index_(off)
	{
	}

	safe_iterator(T& s, decltype(std::declval<T>( ).data( )) ptr)
		:safe_iterator(s, std::distance(s.data( ), ptr))
	{
	}

	template<typename T1 = T>
		requires(std::assignable_from<T*, T1*>)
	safe_iterator(const safe_iterator<T1> other)
		: source_(other.source_), index_(other.index_)
	{
	}

	auto operator->( ) const noexcept
	{
		return get_ptr(source_->data( )[index_]);
	}

	auto& operator*( ) const noexcept
	{
		return get_ref(source_->data( )[index_]);
	}

	bool operator!( ) const noexcept
	{
		return source_ == nullptr;
	}

	explicit operator bool( ) const noexcept
	{
		return source_ != nullptr;
	}

	bool operator==(const safe_iterator other) const noexcept
	{
		return source_ == other.source_ && index_ == other.index_;
	}

	// Prefix increment
	safe_iterator& operator++( ) noexcept
	{
		++index_;
		return *this;
	}

	// Postfix increment
	safe_iterator operator++(int) noexcept
	{
		const auto tmp = *this;
		++index_;
		return tmp;
	}

private:
	T* source_ = nullptr;
	size_t index_ = static_cast<size_t>(-1);
};

namespace std
{
	template<typename T, typename T1>
	auto distance(const T* ptr, const safe_iterator<T1> itr)
	{
		return std::distance(ptr, itr.data( ));
	}

	template<typename T, typename T1>
	auto distance(const safe_iterator<T> itr, const T1* ptr)
	{
		return std::distance(itr.data( ), ptr);
	}

	template<typename T, typename T1>
	auto distance(const safe_iterator<T> itr, const safe_iterator<T1> itr1)
	{
		return std::distance(itr.data( ), itr1.data( ));
	}
}

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
		if (std::holds_alternative<size_t>(getter_))
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
		data_.emplace_back(std::move(uptr));
#ifdef _DEBUG
		validate_item(ret);
#endif
		return ret;
	}

public:
	using data_type = std::vector<std::unique_ptr<basic_netvar_info>>;
	using iterator = safe_iterator<const data_type>;

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
	data_type data_;
};

export namespace cheat::netvars
{
	class basic_storage
	{
	public:
		using data_type = std::vector<netvar_table>;
		using iterator = safe_iterator<data_type>;
		using const_iterator = safe_iterator<const data_type>;

		[[deprecated]]
		bool contains_duplicate(const nstd::hashed_string_view name, netvar_table* const from = nullptr) const noexcept;

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
		data_type data_;
	};
}