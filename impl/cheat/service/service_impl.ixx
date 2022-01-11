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

		template<std::derived_from<basic_service> S>
		static auto& get(S* holder)
		{
			const auto deps = holder->_Deps<false>( );
			const auto index = index_holder::get( ).index;
			return deps[index];
		}

		template<std::derived_from<basic_service> E>
		static auto unwrap(E* element)//convert element from basic_service to T
		{
			const auto basic = static_cast<const basic_service*>(element);
			const auto addr = reinterpret_cast<uintptr_t>(basic) + index_holder::get( ).offset;
			using ret_v = std::conditional_t<std::is_const_v<E>, const T, T>;
			return reinterpret_cast<ret_v*>(addr);
		}

		template<class S>
		static auto get_unwrap(S* holder)
		{
			auto& val = get(holder);
			using ptr_t = std::conditional_t<std::is_const_v<S>, const basic_service, basic_service>;
			return unwrap<ptr_t>(val.get( ));
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
		void add(std::shared_ptr<T> sptr = {})const
		{
			//provide pointer manually to root service
			//othervise take it from there

			if (!sptr)//this code cannot be called from a constructor
			{
				auto rootptr = get_root_service( );
#ifdef _DEBUG
				auto deps = rootptr->_Deps<true>( );
				if (std::ranges::find(deps, typeid(T), &basic_service::type) == deps.end( ))
					runtime_assert("Service not registered");
#endif
				auto& asptr = service_getter<T, services_loader>::get(rootptr);
				sptr = std::dynamic_pointer_cast<T>(asptr);
				//sptr = service_getter<T, services_loader>::get(rootptr);
			}

			size_t index = holder->_Add_dependency(std::move(sptr));
			deps_getter<T>::set(index, dynamic_cast<Holder*>(holder));
		}
	};

	template <typename Holder>
	struct service;

	template <typename Holder>
	service_deps_getter(service<Holder>*)->service_deps_getter<Holder, false>;
	template <typename Holder>
	service_deps_getter(const service<Holder>*)->service_deps_getter<Holder, true>;

	template <typename Holder>
	struct service : basic_service
	{
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

		auto deps( )
		{
			return service_deps_getter(this);
		}

		auto deps( )const
		{
			return service_deps_getter(this);
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