module;

#include "basic_includes.h"

export module cheat.service;
export import :basic;
export import :tools;
import cheat.tools.object_name;

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

		using _Getter_type = service_getter_index<T, Holder>;
		_Getter_type& _Getter = nstd::one_instance<_Getter_type>::get( );

		bool valid( )const
		{
			return !_Getter.unset( );
		}

		template<std::derived_from<basic_service> H>
		bool valid(const H* holder)const
		{
			if (!valid( ))
				return false;
			const auto& deps = holder->load_before;
			if (deps.size( ) <= _Getter.index)
				return false;
			runtime_assert(deps[_Getter.index]->type( ) == typeid(T));
			return true;
		}

		void set(size_t index, Holder* holder) const
		{
			runtime_assert(!valid( ));
			_Getter.index = index;
			auto base = static_cast<basic_service*>(holder);
			_Getter.offset = std::distance(reinterpret_cast<uint8_t*>(base), reinterpret_cast<uint8_t*>(holder));
		}

		template<std::derived_from<basic_service> H>
		auto& get(H* holder)const
		{
			runtime_assert(valid( ));
			return holder->load_before[_Getter.index];
		}

		/// <summary>
		/// convert element from basic_service to T
		/// </summary>
		/// <param name="element"> element inside holder </param>
		/// <returns></returns>
		template<std::derived_from<basic_service> E>
		auto unwrap(E* element)const
		{
			runtime_assert(valid( ));
			const auto basic = static_cast<const basic_service*>(element);
			const auto addr = reinterpret_cast<uintptr_t>(basic) + _Getter.offset;
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
	};

	template <typename Holder>
	using service_deps_getter_const = basic_service_deps_getter<Holder, true>;

	class services_cache_impl
	{
	public:
		using deps_storage = basic_service::deps_storage;
		using value_type = deps_storage::value_type;

		template<typename T>
		value_type try_access( )const
		{
			const auto lock = std::scoped_lock(mtx_);
			auto item = storage_.find(typeid(T));
			return item != storage_.end( ) ? *item : value_type( );
		}

		template<typename T>
		/*std::conditional_t<std::ranges::random_access_range<storage_type>, value_type, value_type&>*/value_type access( )
		{
			const auto lock = std::scoped_lock(mtx_);
			auto item = storage_.find(typeid(T));
			return item != storage_.end( ) ? *item : storage_[storage_.add<T>( )];
		}

		void store(value_type&& val) = delete;
		[[deprecated]]
		void store(const value_type& val)
		{
			const auto lock = std::scoped_lock(mtx_);
			if (storage_.contains(val->type( )))
				return;
			storage_.add(val);
		}

	private:
		mutable std::recursive_mutex mtx_;
		deps_storage storage_;
	};

	using services_cache = nstd::one_instance<services_cache_impl>;

	template <typename Holder, class Base = basic_service_deps_getter<Holder, false>>
	struct service_deps_getter :Base
	{
		using Base::Base;
		using Base::holder;

		template<typename T>
		T* add(bool can_be_skipped = false)const
		{
			services_cache_impl::value_type item;
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
			size_t index = holder->load_before.add(std::move(item));
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
			return tools::object_name<Holder>( );
		}
		const std::type_info& type( ) const final
		{
			return typeid(Holder);
		}

		service_deps_getter<Holder> deps( )
		{
			return this;
		}
		service_deps_getter_const<Holder> deps( )const
		{
			return this;
		}
	};

	template<typename Holder>
	struct dynamic_service :service<Holder>
	{
	};

	template<typename Holder>
	struct static_service :service<Holder>, nstd::one_instance<Holder>
	{
	};
}