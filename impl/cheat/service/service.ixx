module;

#include "basic_includes.h"
#include <nstd/type name.h>

export module cheat.service;
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

template<typename Ret, typename Arg1>
struct function_info
{
	using return_type = Ret;
	using value_type = Arg1;
};

template<typename Ret, typename T, typename ...Args>
function_info<Ret, std::remove_const_t<T>> _Function_info_impl(std::function<Ret(T*, Args...)>)
{
	return {};
}

template<typename Fn>
auto _Function_info(Fn&& fn)
{
	return _Function_info_impl(std::function(fn));
}

export namespace cheat
{
	template<typename T, typename Holder>
	struct service_getter_index
	{
		static constexpr size_t _Default = static_cast<size_t>(-1);

		size_t index = _Default;
		size_t offset = _Default;
		bool unset( )const
		{
			return index == _Default || offset == _Default;
		}
	};

	template<typename T, typename Holder>
	struct service_getter
	{
		//using value_type = /*typename Holder*/basic_service::value_type;

		[[no_unique_address]] nstd::one_instance<service_getter_index<T, Holder>> index_holder;

		void set(size_t index, Holder* holder) const
		{
			auto& ref = index_holder.get( );
			runtime_assert(ref.unset( ));
			ref.index = index;
			auto base = static_cast<basic_service*>(holder);
			ref.offset = std::distance(reinterpret_cast<uint8_t*>(base), reinterpret_cast<uint8_t*>(holder));
		}

		template<std::derived_from<basic_service> H>
		auto& get(H* holder)const
		{
			auto& idx_holder = index_holder.get( );
			runtime_assert(!idx_holder.unset( ));
			const auto deps = holder->_Deps<false>( );
			return deps[idx_holder.index];
		}

		bool valid( )const
		{
			return !index_holder.get( ).unset( );
		}

		template<std::derived_from<basic_service> H>
		bool valid(const H* holder)const
		{
			auto& idx_holder = index_holder.get( );
			if (idx_holder.unset( ))
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
			runtime_assert(!idx_holder.unset( ));
			const auto basic = static_cast<const basic_service*>(element);
			const auto addr = reinterpret_cast<uintptr_t>(basic) + idx_holder.offset;
			return reinterpret_cast<nstd::add_const_if<E, T*>>(addr);
		}

		template<class H>
		auto get_unwrap(H* holder)const
		{
			auto& val = get(holder);
			return unwrap<nstd::add_const_if<H, basic_service>>(val.get( ));
		}
	};

	template <typename Holder, bool Const>
	class basic_service_deps_getter
	{
	public:
		using value_type = nstd::add_const_if_v<Const, basic_service>*;
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

		template<typename Fn, typename...Args>
		auto try_call(Fn fn, Args&&...args)const
		{
			using fn_info = decltype(_Function_info(fn));
			using ret_t = fn_info::return_type;

			auto ptr = try_get<fn_info::value_type>( );
			const auto valid_ptr = ptr != nullptr;

			const auto invoke = [&]( )->decltype(auto)
			{
				return std::invoke(fn, ptr, std::forward<Args>(args)...);
			};

			if constexpr (std::is_void_v<ret_t>)
			{
				bool ret = false;
				if (valid_ptr)
				{
					ret = true;
					invoke( );
				}
				return ret;
			}
			else
			{
				std::optional<ret_t> ret;
				if (valid_ptr)
					ret.emplace(invoke( ));
				return ret;
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

	class services_cache_impl
	{
		using storage_type = basic_service::deps_storage;
		using value_type = basic_service::value_type;

		mutable std::recursive_mutex mtx_;
		storage_type storage_;

		template<typename T>
		static auto find(T& storage, const std::type_info& type)
		{
			/*return std::ranges::find_if(storage, [&](value_type& val)
										{
											return val->type( ) == type;
										});*/
			auto end = storage.end( );
			for (auto itr = storage.begin( ); itr != end; ++itr)
			{
				if ((*itr)->type( ) == type)
					return itr;
			}
			return end;
		}
	public:

		template<typename T>
		value_type try_access( )const
		{
			const auto lock = std::scoped_lock(mtx_);
			auto item = find(storage_, typeid(T));
			return item != storage_.end( ) ? *item : value_type( );
		}

		template<typename T>
		std::conditional_t<std::ranges::random_access_range<storage_type>, value_type, value_type&> access( )
		{
			const auto lock = std::scoped_lock(mtx_);
			auto item = find(storage_, typeid(T));
			return item != storage_.end( ) ? *item : storage_.emplace_back(basic_service::_Create<T>( ));
		}

		auto store(const value_type& val)
		{
			const auto lock = std::scoped_lock(mtx_);
			const auto item = find(storage_, val->type( ));
			if (item != storage_.end( ))
				return;
			storage_.emplace_back(val);
		}
	};
	using services_cache = nstd::one_instance<services_cache_impl>;

	template <typename Holder>
	struct service_deps_getter<Holder, false> : basic_service_deps_getter<Holder, false>
	{
		using _Base = basic_service_deps_getter<Holder, false>;
		using _Base::_Base;
		using _Base::holder;

		template<typename T>
		T* add(bool can_be_skipped = false)const
		{
			basic_service::value_type item;
			if (!can_be_skipped)
			{
				item = services_cache::get( ).access<T>( );
			}
			else
			{
				item = services_cache::get( ).try_access<T>( );
				if (!item)
					return nullptr;
			}

			service_getter<T, Holder> getter;
			T* ret = dynamic_cast<T*>(item.get( ));
			size_t index = holder->_Add_dependency(std::move(item));
			getter.set(index, dynamic_cast<Holder*>(holder));
			return ret;
		}
	};

	//-----

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