module;

#include "basic_includes.h"

#include <nstd/type_traits.h>

export module cheat.service:basic;

template<typename Ret, typename ...Args>
struct shared_function :std::shared_ptr<std::function<Ret(Args...)>>
{
	shared_function( ) = default;

	template<typename Fn>
	shared_function(const Fn fn)
	{
		using fn_t = std::function<Ret(Args...)>;
		*static_cast<std::shared_ptr<fn_t>*>(this) = std::make_shared<fn_t>(fn);
	}

	Ret operator()(Args ...args) const
	{
		if (!*this)
		{
			if constexpr (std::is_void_v<Ret>)
				return;
			else if constexpr (std::default_initializable<Ret>)
				return Ret( );
			else
				throw std::logic_error("Unable to make default value");
		}
		return std::invoke(**this, std::forward<Args>(args)...);
	}
};

template<typename T>
struct shared_object_abstract : std::shared_ptr<T>
{
	shared_object_abstract( )
		:std::shared_ptr<T>(nullptr)
	{
		static_assert(std::is_abstract_v<T>, "T must be abstract!");
	}

	template<typename E>
	shared_object_abstract(std::type_identity<E>)
		: std::shared_ptr<T>(std::make_shared<E>( ))
	{
		static_assert(std::is_abstract_v<T>, "T must be abstract!");
	}
};

template<typename T>
struct shared_object_simple : std::shared_ptr<T>
{
	shared_object_simple( )
		:std::shared_ptr<T>(std::make_shared<T>( ))
	{
		static_assert(!std::is_abstract_v<T>, "T is abstract!");
	}
};

template<typename T>
using shared_object = std::conditional_t<std::is_abstract_v<T>, shared_object_abstract<T>, shared_object_simple<T>>;

template<typename T>
auto& deref(T& obj) noexcept
{
	if constexpr (std::_Dereferenceable<T>)
		return deref(*obj);
	else
		return obj;
}

template<typename T, class Vec = std::vector<T>>
struct wrapped_vector :protected Vec
{
	using Vec::Vec;
	using Vec::operator=;
	using Vec::begin;
	using Vec::end;
	using Vec::empty;
	using Vec::size;
	using Vec::operator [];

	using typename Vec::value_type;
	using typename Vec::size_type;
	using typename Vec::iterator;
	using typename Vec::const_iterator;

	size_type add(T&& val)
	{
		runtime_assert(!contains(val->type( )), "Element already stored!");
		Vec::push_back(std::move(val));
		return size( ) - 1;
	}

	size_type add(const T& val)
	{
		runtime_assert(!contains(val->type( )), "Element already stored!");
		Vec::push_back(val);
		return size( ) - 1;
	}

	template<class C>
	size_type add( )
	{
		return add(std::type_identity<C>{});
	}

	iterator find(const std::type_info& type) noexcept
	{
		const auto ed = end( );
		for (auto itr = begin( ); itr != ed; ++itr)
		{
			if (deref(itr).type( ) == type)
				return itr;
		}
		return ed;
	}

	const_iterator find(const std::type_info& type) const noexcept
	{
		return const_cast<wrapped_vector*>(this)->find(type);
	}

	bool contains(const std::type_info& type) const noexcept
	{
		return find(type) != end( );
	}
};

export namespace cheat
{
	class __declspec(novtable) basic_service
	{
	public:
		enum class state_type :uint8_t
		{
			idle
			, started
			, loaded
			, loaded_error
		};

		using executor = shared_object<cppcoro::static_thread_pool>;
		using task_type = cppcoro::shared_task<bool>;
		using deps_storage = wrapped_vector<shared_object_abstract<basic_service>>;
		using callback_type = shared_function<void, basic_service*, state_type>;

		std::atomic<state_type> state = state_type::idle;
		task_type start_result;
		deps_storage load_before;

		basic_service( );
		virtual ~basic_service( );

		basic_service(const basic_service& other) = delete;
		basic_service(basic_service&& other) noexcept;
		basic_service& operator=(const basic_service& other) = delete;
		basic_service& operator=(basic_service&& other) noexcept;

		virtual std::string_view name( ) const = 0;
		virtual const std::type_info& type( ) const = 0;

	private:
		task_type load_deps(const executor ex, const callback_type callback) noexcept;
		task_type load_this(const executor ex, const callback_type callback) noexcept;
		task_type start_impl(const executor ex, const callback_type callback) noexcept;

	public:
		void start(const executor ex, const callback_type callback) noexcept;
		void start(const executor ex) noexcept;
		void start( ) noexcept;

	protected:
		//delayed constructor
		virtual void construct( )noexcept = 0;
		virtual bool load( ) noexcept
		{
			return true;
		}
	};
}
