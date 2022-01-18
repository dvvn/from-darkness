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

template <bool Val, typename T>
using add_const_if_v = std::conditional_t<Val, std::add_const_t<T>, T>;

template <typename Test, typename T>
using add_const_if = add_const_if_v<std::is_const_v<Test>, T>;

export namespace cheat
{
	template<typename T, typename Holder>
	struct service_getter_index
	{
		size_t index;
		size_t offset;
		bool unset = true;
	};

	template<typename T, typename Holder>
	struct service_getter
	{
		//using value_type = /*typename Holder*/basic_service::value_type;

		[[no_unique_address]] nstd::one_instance<service_getter_index<T, Holder>> index_holder;

		void set(size_t index, Holder* holder) const
		{
			auto& ref = index_holder.get( );
			runtime_assert(ref.unset);
			ref.index = index;
			auto base = static_cast<basic_service*>(holder);
			ref.offset = std::distance(reinterpret_cast<uint8_t*>(base), reinterpret_cast<uint8_t*>(holder));
			ref.unset = false;
		}

		template<std::derived_from<basic_service> H>
		auto& get(H* holder)const
		{
			auto& idx_holder = index_holder.get( );
			runtime_assert(!idx_holder.unset);
			const auto deps = holder->_Deps<false>( );
			return deps[idx_holder.index];
		}

		bool valid( )const
		{
			return !index_holder.get( ).unset;
		}

		template<std::derived_from<basic_service> H>
		bool valid(const H* holder)const
		{
			auto& idx_holder = index_holder.get( );
			if (idx_holder.unset)
				return false;
			const auto deps = holder->_Deps<false>( );
			if (deps.size( ) <= idx_holder.index)
				return false;
			runtime_assert(deps[idx_holder.index]->type( ) == typeid(T));
			return true;
		}

		/// <summary>
		/// convert element from basic_service to T
		/// </summary>
		/// <param name="element"> element inside holder </param>
		/// <returns></returns>
		template<std::derived_from<basic_service> E>
		auto unwrap(E* element)const
		{
			auto& idx_holder = index_holder.get( );
			runtime_assert(!idx_holder.unset);
			const auto basic = static_cast<const basic_service*>(element);
			const auto addr = reinterpret_cast<uintptr_t>(basic) + idx_holder.offset;
			return reinterpret_cast<add_const_if<E, T*>>(addr);
		}

		template<class H>
		auto get_unwrap(H* holder)const
		{
			auto& val = get(holder);
			return unwrap<add_const_if<H, basic_service>>(val.get( ));
		}
	};

	class services_loader;
	basic_service* get_root_service( );

	///if wont export code before singleton with service_getter will be corrupted inside shared library
	//----
	//----
	//----

	template <typename Holder, bool Const>
	class basic_service_deps_getter
	{
	public:
		using value_type = add_const_if_v<Const, basic_service>*;
		using holder_type = Holder;

		basic_service_deps_getter(value_type h) :holder(h)
		{
		}

	protected:
		value_type holder;
	public:

		template<typename T>
		bool valid( )const
		{
			service_getter<T, Holder> getter;
			return getter.valid(
#ifdef _DEBUG
				holder
#endif
			);
		}

		template<typename T>
		const auto& share( )const
		{
			service_getter<T, Holder> getter;
			return getter.get(holder);
		}

		template<typename T>
		std::shared_ptr<T> try_share(bool force_valid = false)const
		{
			return (force_valid || valid<T>( )) ? std::dynamic_pointer_cast<T>(share<T>( )) : nullptr;
		}

		template<typename T>
		auto get_ptr( )const
		{
			service_getter<T, Holder> getter;
			return getter.get_unwrap(holder);
		}

		template<typename T>
		auto& get( )const
		{
			return *get_ptr<T>( );
		}

		template<typename T>
		auto try_get( )const
		{
			return valid<T>( ) ? get_ptr<T>( ) : nullptr;
		}

		template<typename T, typename Fn>
		decltype(auto) try_call(Fn fn)const
		{
			using val_t = add_const_if_v<Const, T>*;
			using ret_t = std::invoke_result_t<Fn, val_t>;

			if constexpr (std::is_void_v<ret_t> || std::default_initializable<ret_t>)
			{
				auto ptr = try_get<T>( );
				if (ptr)
					return std::invoke(fn, ptr);

				if constexpr (!std::is_void_v<ret_t>)
					return ret_t{};
			}
			else
			{
				runtime_assert(try_get<T>( ) != nullptr, "Unable to create default value!");
				return std::invoke(fn, get_ptr<T>( ));
			}
		}
	};

	template <typename Holder, bool Const>
	struct service_deps_getter;

	template <typename Holder>
	struct service_deps_getter<Holder, true> : basic_service_deps_getter<Holder, true>
	{
		using basic_service_deps_getter<Holder, true>::basic_service_deps_getter;
	};

	_INLINE_VAR bool service_deps_getter_add_allow_skip = true;

	template <typename Holder, class Base = basic_service_deps_getter<Holder, false>>
	struct service_deps_getter_rw : Base
	{
		using typename Base::value_type;
		using Base::holder;

		service_deps_getter_rw(value_type h) :Base(h)
		{
		}

		template<typename T>
		T* add(std::shared_ptr<T>&& sptr)const
		{
			service_getter<T, Holder> getter;
			if (service_deps_getter_add_allow_skip && !sptr)
				return nullptr;

			T* ret = sptr.get( );
			size_t index = holder->_Add_dependency(std::move(sptr));
			getter.set(index, dynamic_cast<Holder*>(holder));
			return ret;
		}
	};

	template <>
	struct service_deps_getter<services_loader, false> : service_deps_getter_rw<services_loader>
	{
		using service_deps_getter_rw::service_deps_getter_rw;
		using service_deps_getter_rw::add;

		template<typename T>
		auto add( )const
		{
			auto sptr = std::make_shared<T>( );
			return this->add(std::move(sptr));
		}
	};

	template <typename Holder>
	struct service_deps_getter<Holder, false> : service_deps_getter_rw<Holder>
	{
		using _Base = service_deps_getter_rw<Holder>;
		using _Base::service_deps_getter_rw;
		using _Base::add;

		template<typename T>
		auto add( )const
		{
			service_deps_getter<services_loader, true> root_getter = get_root_service( );
			auto sptr = root_getter.try_share<T>(service_deps_getter_add_allow_skip);
			return this->add(std::move(sptr));
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
			return this;
		}
		service_deps_getter<Holder, true> deps( )const
		{
			return this;
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