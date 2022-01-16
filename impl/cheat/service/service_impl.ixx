module;

#include "includes.h"
#include <nstd/type name.h>

export module cheat.service:impl;
export import :basic;

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

template <typename T, typename V>
using add_const_if = std::conditional_t<std::is_const_v<T>, std::add_const_t<V>, V>;

export namespace cheat
{
	template<typename T, typename Holder>
	struct service_getter_index
	{
		using value_type = T;

		size_t index;
		size_t offset;
		bool unset = true;
	};

	template<typename T, typename Holder>
	class service_getter
	{
	public:
		service_getter( ) = delete;
		using index_holder_type = service_getter_index<T, Holder>;
		using index_holder = nstd::one_instance<index_holder_type>;
		using value_type = /*typename Holder*/basic_service::value_type;

		static void set(size_t index, Holder* holder)
		{
			auto& ref = index_holder::get( );
			runtime_assert(ref.unset);
			ref.index = index;
			auto base = static_cast<basic_service*>(holder);
			ref.offset = std::distance(reinterpret_cast<uint8_t*>(base), reinterpret_cast<uint8_t*>(holder));
			ref.unset = false;
		}

		template<std::derived_from<basic_service> H>
		static auto& get(H* holder)
		{
			auto& idx_holder = index_holder::get( );
			runtime_assert(!idx_holder.unset);
			const auto deps = holder->_Deps<false>( );
			return deps[idx_holder.index];
		}

		static bool valid( )
		{
			return !index_holder::get( ).unset;
		}

		template<std::derived_from<basic_service> H>
		static bool valid(const H* holder)
		{
			auto& idx_holder = index_holder::get( );
			if (idx_holder.unset)
				return false;
			const auto deps = holder->_Deps<false>( );
			if (deps.size( ) <= idx_holder.index)
				return false;
			runtime_assert(deps[idx_holder.index].type( ) == typeid(index_holder_type::value_type));
			return true;
		}

		/// <summary>
		/// convert element from basic_service to T
		/// </summary>
		/// <param name="element"> element inside holder </param>
		/// <returns></returns>
		template<std::derived_from<basic_service> E>
		static auto unwrap(E* element)
		{
			auto& idx_holder = index_holder::get( );
			runtime_assert(!idx_holder.unset);
			const auto basic = static_cast<const basic_service*>(element);
			const auto addr = reinterpret_cast<uintptr_t>(basic) + idx_holder.offset;
			return reinterpret_cast<add_const_if<E, T*>>(addr);
		}

		template<class H>
		static auto get_unwrap(H* holder)
		{
			auto& val = get(holder);
			return unwrap<add_const_if<H, basic_service>>(val.get( ));
		}
	};

	template <typename Holder, bool Const>
	struct service_deps_getter;

	template <typename Holder>
	struct service_deps_getter<Holder, true>
	{
		template<typename T>
		using deps_getter = service_getter<T, Holder>;
		const basic_service* holder;

		template<typename T>
		auto& share( )const
		{
			return deps_getter<T>::get(holder);
		}

		template<typename T>
		const T& get( )const
		{
			return *deps_getter<T>::get_unwrap(holder);
		}
	};

	class services_loader;
	basic_service* get_root_service( );

	template <typename Holder>
	struct service_deps_getter<Holder, false>
	{
		template<typename T>
		using deps_getter = service_getter<T, Holder>;
		basic_service* holder;

		template<typename T>
		const auto& share( )const
		{
			return deps_getter<T>::get(holder);
		}

		template<typename T>
		T& get( )const
		{
			return *deps_getter<T>::get_unwrap(holder);
		}

		template<typename T>
		T* try_get( )const
		{
			if (!deps_getter<T>::valid(
#ifdef _DEBUG
				holder
#endif
				))
			{
				return nullptr;
			}
			return deps_getter<T>::get_unwrap(holder);
		}

		template<typename T, typename Fn>
		decltype(auto) try_call(Fn fn)const
		{
			using ret_t = std::invoke_result_t<Fn, T*>;
			if constexpr (std::is_void_v<ret_t>)
			{
				T* ptr = try_get<T>( );
				if (ptr)
					std::invoke(fn, ptr);
			}
			else if constexpr (std::default_initializable<ret_t>)
			{
				T* ptr = try_get<T>( );
				ret_t ret = {};
				if (ptr)
					ret = std::invoke(fn, ptr);
				return ret;
			}
			else
			{
				T* ptr = std::addressof(get<T>( ));
				return std::invoke(fn, ptr);
			}
		}

		template<typename T>
		void add(std::shared_ptr<T>&& sptr)const
		{
			size_t index = holder->_Add_dependency(std::move(sptr));
			deps_getter<T>::set(index, dynamic_cast<Holder*>(holder));
		}

		template<typename T>
		void add( )const
		{
			auto rootptr = get_root_service( );

			auto deps = rootptr->_Deps<true>( );
			auto loaded = std::ranges::find(deps, typeid(T), &basic_service::type);
			if (loaded == deps.end( ))
				return;

			auto& asptr = service_getter<T, services_loader>::get(rootptr);
			auto sptr = std::dynamic_pointer_cast<T>(asptr);
			//sptr = service_getter<T, services_loader>::get(rootptr);

			add(std::move(sptr));
		}
	};

	template <typename Holder>
	struct service : basic_service
	{
		std::string_view name( ) const final
		{
			return service_name<Holder>.view( );
		}
		const std::type_info& type( ) const final
		{
			return typeid(Holder);
		}

		service_deps_getter<Holder, false> deps( )
		{
			return {static_cast<basic_service*>(this)};
		}
		service_deps_getter<Holder, true> deps( )const
		{
			return {static_cast<const basic_service*>(this)};
		}
	};

	template<typename Holder>
	struct dynamic_service :service<Holder>
	{
	private:
		using basic_service::_Deps;
	};

	template<typename Holder>
	struct static_service :service<Holder>, nstd::one_instance<Holder>
	{
	};
}