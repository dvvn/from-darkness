module;

#include <nstd/type name.h>

#include "includes.h"

export module cheat.service:core;

//constexpr auto _Fix_service_name(const std::string_view& name)
//{
//	auto str = nstd::drop_namespace(name, "cheat");
//	constexpr std::string_view dummy = "_impl";
//	if (str.ends_with(dummy))
//		str.erase(str.end( ) - dummy.size( ), str.end( ));
//	return str;
//}

template <typename T>
_INLINE_VAR constexpr auto service_name = []
{
	constexpr auto raw = nstd::type_name<T>( );
#if 1
	constexpr auto buffer = nstd::drop_namespace(raw, "cheat");
	if constexpr (buffer.ideal( ))
		return buffer;
	else
		return buffer.make_ideal<buffer.str_size>( );
#else
	const auto raw_str = _Fix_service_name(raw);
#if 1
	constexpr auto buff_size = _Fix_service_name(raw).size( );
	auto buff = nstd::string_to_buffer<buff_size>(raw_str);
#else
	auto buff = nstd::string_to_buffer<raw_str.size( )>(raw_str);
#endif
	return buff;
#endif
}();

export namespace cheat
{
	enum class service_state : uint8_t
	{
		unset = 0
		, waiting
		, loading
		, loaded
		, error
	};

	/*class basic_service;

	template <typename T>
	concept root_service = std::derived_from<T, basic_service>;*/

	//template </*root_service*/class T>
	//class shared_service;

	class __declspec(novtable) basic_service
	{
	public:
		using executor = cppcoro::static_thread_pool;
		using load_result = cppcoro::task<bool>;
		using mutex_type = cppcoro::async_mutex;
		using value_type = std::shared_ptr<basic_service>;
		using deps_storage = std::vector<value_type>;

		basic_service( );
		virtual ~basic_service( );

		basic_service(const basic_service& other) = delete;
		basic_service(basic_service&& other) noexcept;
		basic_service& operator=(const basic_service& other) = delete;
		basic_service& operator=(basic_service&& other) noexcept;

		const value_type* find(const std::type_info& info) const;
		value_type* find(const std::type_info& info);
		//void erase(const std::type_info& info);
		//using erase_pred = std::function<bool(const value_type&)>;
		//void erase(const erase_pred& fn);
		//void erase_all(const erase_pred& fn);
		void unload( );

	protected:
		size_t add_dependency(value_type&& srv);
		/*template <root_service T>
		size_t add_dependency(const shared_service<T>& srv)
		{
			return add_dependency(srv.share( ));
		}*/
	public:

		virtual std::string_view name( ) const = 0;
		virtual const std::type_info& type( ) const = 0;

		service_state state( ) const;
		//rename to 'do load'
		load_result load(executor& ex) noexcept;
	private:
		void set_state(service_state state);
	public:

#if defined(_DEBUG) && _ITERATOR_DEBUG_LEVEL != 0
#define CHEAT_SERVICE_AT at
#else
#define CHEAT_SERVICE_AT operator[]
#endif

		value_type& at(size_t index)
		{
			return deps_.CHEAT_SERVICE_AT(index);
		}

		const value_type& at(size_t index)const
		{
			return deps_.CHEAT_SERVICE_AT(index);
		}

	protected:
		//rename to 'construct' 'lazy load' or something like it
		virtual void load_async( )noexcept = 0;
		//rename to 'load'
		virtual bool load_impl( ) noexcept
		{
			return true;
		}

		std::span<value_type>deps( ) { return deps_; }
		std::span<const value_type>deps( )const { return deps_; }

	private:
		mutex_type lock_;
		deps_storage deps_;
		service_state state_ = service_state::unset;
	};

#if 0
	template </*root_service*/class T>
	class shared_service
	{
	public:
		using element_type = T;
		using shared_type = stored_service<T>;

		shared_service(const shared_service& other) = delete;
		shared_service& operator=(const shared_service& other) = delete;

		shared_service(shared_service&& other) noexcept
		{
			obj_ = std::move(other.obj_);
#ifndef _DEBUG
			other.obj_ = nullptr;
#endif
		}

		shared_service& operator=(shared_service&& other) noexcept
		{
			std::swap(obj_, other.obj_);
			return *this;
		}

		template <typename ...Args>
		shared_service(Args&&...args)
		{
			auto obj = std::make_shared<T>(std::forward<Args>(args)...);
			obj_ = obj
#ifndef _DEBUG
				.get( )
#endif
				;
			add_to_loader(std::move(obj));
		}

		~shared_service( )
		{
#ifdef _DEBUG
			if (obj_.expired( ))
				return;
#endif
			runtime_assert(get_from_loader(type( )) == nullptr, "shared service destroyed before loader!");
		}

		static const std::type_info& type( ) { return typeid(T); }

		_NODISCARD shared_type share( ) const
		{
			auto stored = get_from_loader(type( ));
			runtime_assert(stored != nullptr, "unable to share service!");
			return std::dynamic_pointer_cast<T>(*stored);
		}

	private:
		_NODISCARD
#ifdef _DEBUG
			shared_type _Get( ) const { return shared_type(obj_); }
#else
			T* _Get( ) const noexcept { return obj_; }
#endif

	public:
		auto operator->( ) const { return _Get( ); }
		[[deprecated]]
		_NODISCARD T& operator*( ) const { return *_Get( ); }

	private:
#ifdef _DEBUG
		std::weak_ptr<T>
#else
		T*
#endif
			obj_;
	};
#endif
}

export namespace cheat
{
	class services_loader;
	basic_service* get_root_service( );
}

export namespace cheat
{
	template<typename ...Args>
	struct service_getter_index
	{
		static constexpr size_t default_value = static_cast<size_t>(-1);
		size_t index = default_value;
		size_t offset = default_value;
	};

	template<typename T, typename Holder>
	class service_getter
	{
		using _Holder = basic_service/*Holder*/;
	public:
		service_getter( ) = delete;

		using index_holder = nstd::one_instance<service_getter_index<T, Holder>>;
		using value_type = /*typename Holder*/basic_service::value_type;

		static void set(size_t index, Holder* holder)
		{
			auto& ref = index_holder::get( );
			runtime_assert(ref.index == ref.default_value);
			runtime_assert(ref.offset == ref.default_value);
			ref.index = index;
			auto base = static_cast<basic_service*>(holder);
			ref.offset = std::distance(reinterpret_cast<uint8_t*>(base), reinterpret_cast<uint8_t*>(holder));
			__noop;
		}

		static const value_type& get(const _Holder* holder)
		{
			return holder->at(index_holder::get( ).index);
		}

		static value_type& get(_Holder* holder)
		{
			return holder->at(index_holder::get( ).index);
		}

		static T* unwrap(basic_service* base)
		{
			auto addr = reinterpret_cast<uintptr_t>(base) + index_holder::get( ).offset;
			return reinterpret_cast<T*>(addr);
		}
	};

	template <typename Holder>
	struct service : basic_service
	{
		template<typename T>
		using deps_getter = service_getter<T, Holder>;

		std::string_view name( ) const final
		{
			return service_name<Holder>.view( );
			/*constexpr auto tmp   = nstd::type_name<T, "cheat">;
			constexpr auto dummy = std::string_view("_impl");
			if constexpr (tmp.ends_with(dummy))
				return tmp.substr(0, tmp.size( ) - dummy.size( ));
			else
				return tmp;*/
		}

		const std::type_info& type( ) const final
		{
			return typeid(Holder);
		}

		template<typename T>
		auto& share_dependency( )const
		{
			return deps_getter<T>::get(this);
		}

		/*template<typename Q, bool Cast = false>
		auto& get_dependency( )
		{
			auto& sptr = deps_getter<Q>::get(this);
			auto ptr = sptr.get( );
			if constexpr (Cast)
				return *dynamic_cast<Q*>(ptr);
			else
				return *ptr;
		}*/

		template<typename T>
		T& get_dependency( )
		{
			auto& sptr = deps_getter<T>::get(this);
			auto ptr = sptr.get( );
			return *deps_getter<T>::unwrap(ptr);
		}

		template<typename T>
		const T& get_dependency( )const
		{
			return std::_Const_cast(this)->get_dependency<T>( );
		}

		template<typename T>
		void add_dependency(std::shared_ptr<T> sptr = {})
		{
			//provide pointer manually to root service
			//othervise take it from there

			if (!sptr)
			{
				auto root = get_root_service( );
				//this cannot be called from a constructor
				runtime_assert(root->find(typeid(T)), "Service not registered");
				auto& asptr = service_getter<T, services_loader>::get(root);
				sptr = std::dynamic_pointer_cast<T>(asptr);
				//sptr = service_getter<T, services_loader>::get(root);
			}

			size_t index = basic_service::add_dependency(std::move(sptr));
			deps_getter<T>::set(index, dynamic_cast<Holder*>(this));
		}

	};

	template<typename T>
	struct dynamic_service :service<T>
	{
	private:
		using service<T>::deps;
	};

	template<typename T>
	struct static_service :service<T>, nstd::one_instance<T>
	{
	};
}
