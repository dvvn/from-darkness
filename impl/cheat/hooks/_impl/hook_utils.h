#pragma once

#include "hook.h"

//#include <boost/timer/timer.hpp>
//#include <boost/thread.hpp>

namespace cheat::hooks
{
	template <typename C>
	LPVOID* _Pointer_to_virtual_class_table(C* instance)
	{
		return *(LPVOID**)instance;
	}

	// ReSharper disable CppInconsistentNaming
	enum class call_conversion
	{
		thiscall__,
		cdecl__,
		stdcall__,
		vectorcall__,
		fastcall__
	};
	// ReSharper restore CppInconsistentNaming

	template <typename Ret, call_conversion Call_cvs, typename C, bool Is_const, typename ...Args>
	struct hook_callback;

#pragma region call_cvs_1

	/*old version:
	template <typename Ret, typename C, typename ...Args>
	struct hook_callback<Ret, call_conversion::call_cvs##__, C, false, Args...>
	{
		virtual ~hook_callback() = default;
		virtual Ret __##call_cvs callback(Args ...) = 0;
		virtual Ret __##call_cvs callback_proxy(Args ...) = 0;
	};
	*/

	template <typename T>
	struct hiddent_type
	{
		using value_type = T;
		uintptr_t value;

		hiddent_type( ) = default;

		hiddent_type(void* ptr): value(reinterpret_cast<uintptr_t>(ptr))
		{
		}

		template <typename T1>
		hiddent_type<T1> change_type( ) const
		{
			hiddent_type<T1> ret;
			ret.value = value;
			return ret;
		}

		std::conditional_t<std::is_pointer_v<T>, T, std::add_lvalue_reference_t<T>> unhide( )
		{
			if constexpr (std::is_pointer_v<T>)
				return reinterpret_cast<T>(value);
			else if constexpr (!std::is_lvalue_reference_v<T>)
				return reinterpret_cast<T&>(value);
			else
				return *reinterpret_cast<std::remove_reference_t<T>*>(value);
		}
	};

#define CALL_CVS_STUFF_IMPL(call_cvs)\
    template <typename Ret, typename C, typename ...Args>\
    struct hook_callback<Ret, call_conversion::call_cvs##__, C, false, Args...>\
    {\
        virtual ~hook_callback() = default;\
        virtual Ret __##call_cvs callback_proxy(hiddent_type<Args> ...) = 0;\
    };\
    template <typename Ret, typename C, typename ...Args>\
    struct hook_callback<Ret, call_conversion::call_cvs##__, C, true, Args...>\
    {\
        virtual ~hook_callback() = default;\
        virtual Ret __##call_cvs callback_proxy(hiddent_type<Args> ...) /*const*/ = 0;\
    };\
    \
    template <typename Ret, typename C, typename ...Args>\
    LPVOID pointer_to_class_method(Ret (__##call_cvs C::*fn )(Args ...)      )\
    {\
        return *reinterpret_cast<void**>(utl::addressof(fn));\
    }\
    template <typename Ret, typename C, typename ...Args>\
    LPVOID pointer_to_class_method(Ret (__##call_cvs C::*fn )(Args ...) const)\
    {\
        return *reinterpret_cast<void**>(utl::addressof(fn));\
    }\
    template <typename Ret, typename C, typename ...Args>\
    Ret call_virtual_class_method(Ret (__##call_cvs C::*fn )(Args ...), C* instance, size_t index, Args ...args)\
    {\
		  auto vtable = _Pointer_to_virtual_class_table(instance);\
		  /*auto real_fn = vtable[index];*/\
		  (void*&)fn = vtable[index];\
		  return utl::invoke(fn, instance,  static_cast<Args>(args)...);\
		  /*return utl::invoke(static_cast<Ret(__##call_cvs *)(	  C*, Args...)>(real_fn), instance, static_cast<Args>(args)...);*/\
    }\
    template <typename Ret, typename C, typename ...Args>\
    Ret call_virtual_class_method(Ret (__##call_cvs C::*fn )(Args ...) const, const C* instance, size_t index, Args ...args)\
    {\
		  auto vtable = _Pointer_to_virtual_class_table(instance);\
		  /*auto real_fn = vtable[index];*/\
		  (void*&)fn = vtable[index];\
		  return utl::invoke(fn, instance,  static_cast<Args>(args)...);\
		  /*return utl::invoke(static_cast<Ret(__##call_cvs *)(const C*, Args...)>(real_fn), instance, static_cast<Args>(args)...);*/\
    }

	CALL_CVS_STUFF_IMPL(thiscall)
	CALL_CVS_STUFF_IMPL(fastcall)
	CALL_CVS_STUFF_IMPL(stdcall)
	CALL_CVS_STUFF_IMPL(vectorcall)
	CALL_CVS_STUFF_IMPL(cdecl)
#undef CALL_CVS_STUFF_IMPL

#pragma endregion

	class method_info
	{
	public:
		enum class type
		{
			fn_unknown,
			fn_static,
			fn_member,
			fn_member_virtual,
		};

		using func_type = utl::function<LPVOID( )>;

	protected:
		method_info(type method_type, bool refresh_result, func_type&& func);
		method_info(type method_type, bool refresh_result, const func_type& func);

	public:
		method_info( ) = default;

		type   get_type( ) const;
		LPVOID get( ) const;
		bool   update( );
		bool   updated( ) const;

	private:
		type      type__ = type::fn_unknown;
		bool      refresh_result__ = false;
		func_type updater__;
		LPVOID    result__ = nullptr;

		template <typename T>
		static constexpr size_t Count_pointers_( )
		{
			return std::is_pointer_v<T> ? Count_pointers_<std::remove_pointer_t<T>>( ) + 1 : 0;
		}

	public:
		static method_info make_static(LPVOID func)
		{
			return method_info(type::fn_static, false, [=] { return func; });
		}

		template <typename Fn>
		static method_info make_member(Fn&& func)
		{
			return method_info(type::fn_member, false, [fn = pointer_to_class_method(func)] { return fn; });
		}

		template <typename C>
		static method_info make_member_virtual(C&& instance, size_t index, bool refrest_result = false)
		{
			constexpr bool rvalue = std::is_rvalue_reference_v<decltype(instance)>;
			using raw_t = std::remove_cvref_t<decltype(instance)>;
			constexpr size_t num_pointers = Count_pointers_<raw_t>( );

			func_type instance_getter_fn_temp;
			func_type method_getter_fn;

			if constexpr (num_pointers == 0)
			{
				BOOST_STATIC_ASSERT_MSG(rvalue == false, "Unable to store rvalue reference!");
				instance_getter_fn_temp = [ptr = utl::addressof(instance)] { return ptr; };
			}
			else if constexpr (num_pointers == 2)
			{
				BOOST_STATIC_ASSERT_MSG(rvalue == false, "Unable to store rvalue reference to pointer!");
				instance_getter_fn_temp = [=] { return *instance; };
			}
			else if constexpr (num_pointers == 1)
			{
				if constexpr (rvalue)
				{
					BOOST_ASSERT_MSG(instance != nullptr, "Rvalue pointer must be set!");
					instance_getter_fn_temp = [=] { return instance; };
				}
				else
				{
					//instance can be null or dynamic, so store pointer to it
					instance_getter_fn_temp = [ptr2 = utl::addressof(instance)] { return *ptr2; };
				}
			}
			else
			{
				BOOST_STATIC_ASSERT_MSG(std::_Always_false<raw_t>, __FUNCTION__);
			}

			method_getter_fn = [instance_getter_fn = utl::move(instance_getter_fn_temp), idx = index]
			{
				return _Pointer_to_virtual_class_table(instance_getter_fn( ))[idx];
			};

			return method_info(type::fn_member_virtual, refrest_result, utl::move(method_getter_fn));
		}

		template <typename Fn>
		static method_info make_custom(bool refresh_result, Fn&& func)
		{
			return method_info(type::fn_unknown, refresh_result, utl::forward<Fn>(func));
		}
	};

	class hook_holder_base
	{
	protected:
		virtual ~hook_holder_base( ) = default;
	public:
		virtual bool hook( ) = 0;
		virtual bool unhook( ) = 0;
		virtual void unhook_safe( ) = 0;
		virtual bool unhook_unsafe( ) = 0;
		virtual bool enable( ) = 0;
		virtual bool disable( ) = 0;
		virtual void disable_safe( ) = 0;
		virtual bool hooked( ) const = 0;
		virtual bool enabled( ) const = 0;

		struct info: utl::one_instance<info>
		{
			using holder = utl::atomic<hook_holder_base*>;

			holder last_func = nullptr;
			holder current_func = nullptr;
		};
	};


namespace detail
{
	template <class Ret>
	class lazy_return_value
	{
		utl::optional<Ret> value_;

	public:
		template <typename T>
		void store_value(T&& val)
			requires(std::is_constructible_v<Ret, decltype(val)>)
		{
			value_.emplace(Ret(utl::forward<T>(val)));
		}

		void reset( )
		{
			value_.reset( );
		}

		bool empty( ) const
		{
			return !value_.has_value( );
		}

		Ret get( )
		{
			if constexpr (std::is_copy_constructible_v<Ret>)
				return *value_;
			else
			{
				Ret ret = static_cast<Ret&&>(*value_);
				reset( );
				return static_cast<Ret&&>(ret);
			}
		}
	};

	template < >
	class lazy_return_value<void>
	{
		bool skip_original__ = false;

	public:
		void set_original_called(bool called)
		{
			skip_original__ = called;
		}

		void reset( )
		{
			skip_original__ = false;
		}

		bool empty( ) const
		{
			return skip_original__ == false;
		}
	};

	template <typename Ret, call_conversion Call_cvs, typename C, bool Is_const, typename ...Args>
	class hook_holder_impl: public hook_holder_base,
							protected hook_callback<Ret, Call_cvs, C, Is_const, Args...>

	{
	public:
		static constexpr bool have_return_value = !std::is_void_v<Ret>;
		static constexpr bool is_static = !std::is_class_v<C>;

	protected:
		//using hook_callback_type = hook_callback<Ret, Call_cvs, C, Is_const, Args...>;

		hook_callback<Ret, Call_cvs, C, Is_const, Args...>* cast_hook_callback( )
		{
			return this;
		}

	private:
		static void Set_instance_assert_( )
		{
			BOOST_ASSERT_MSG(this_instance__ == nullptr, "Instance already set!");
		}

		static void Unset_instance_assert_( )
		{
			BOOST_ASSERT_MSG(this_instance__ != nullptr, "Instance is unset!");
		}

		void Unmanaged_call_assert( ) const
		{
			BOOST_ASSERT_MSG(!this_instance__ || this_instance__ == this, "Unable to get 'this' pointer!");
		}

		LPVOID original_func__ = nullptr;

		inline static hook_holder_impl*            this_instance__ = nullptr;
		std::conditional_t<Is_const, const C*, C*> target_instance__ = nullptr;

		struct
		{
			bool unhook = false;
			bool disable = false;
			//-
		} safe__;

	protected:
		virtual void Callback(Args ...) = 0;

		auto Target_instance( ) const
		{
			return target_instance__;
		}

		using return_value_holder = lazy_return_value<Ret>;

		bool                call_original_first_ = false;
		return_value_holder return_value_;

		// ReSharper disable once CppNotAllPathsReturnValue
		Ret callback_proxy_impl(Args ...args)
		{
			Unset_instance_assert_( );
			hook_holder_impl& inst = *this_instance__;
			inst.target_instance__ = is_static ? nullptr : reinterpret_cast<C*>(cast_hook_callback( )); //cast this to current vtable first!

			auto& info = info::get( );
			info.current_func = this_instance__;

			return_value_holder& result = inst.return_value_;
			if (inst.call_original_first_)
				inst.call_original_ex(static_cast<Args>(args)...);
			else
				result.reset( );
			inst.Callback(utl::forward<Args>(args)...);
			if (result.empty( ))
				inst.call_original_ex(static_cast<Args>(args)...);

			info.last_func = this_instance__;
			info.current_func = nullptr;

			if (inst.safe__.unhook)
				inst.unhook( );
			else if (inst.safe__.disable)
				inst.disable( );

			if constexpr (have_return_value)
				return result.get( );
		}

		//call original and store result to use later
		auto call_original_ex(Args ...args)
		{
			Unmanaged_call_assert( );
			if constexpr (have_return_value)
			{
				return_value_.store_value(call_original(static_cast<Args>(args)...));
				if constexpr (std::is_copy_constructible_v<Ret> && std::is_trivially_destructible_v<Ret>)
					return return_value_.get( );
			}
			else
			{
				call_original(static_cast<Args>(args)...);
				return_value_.set_original_called(true);
			}
		}

		method_info target_func_;

		~hook_holder_impl( ) override
		{
			if (this->hooked( ))
			{
#if 0
					bool exception;
					try //thrown if thread not started
					{
						auto fut = unhook_safe(chrono::seconds(20));
						(void)fut;
						fut.wait();
						exception = (fut.has_exception());
					}
					catch (...)
					{
						exception = true;
					}
					if (exception)
#endif
				Unhook_lazy_( );
			}
		}

		void Unhook_lazy_( )
		{
			this->safe__.unhook = true;
			utl::this_thread::sleep_for(utl::chrono::milliseconds(150));
			this->unhook_unsafe( );
		}

	public:
		hook_holder_impl( ) = default;

		hook_holder_impl& operator=(const hook_holder_impl&) = delete;
		hook_holder_impl(const hook_holder_impl& other) = delete;
		hook_holder_impl& operator=(hook_holder_impl&&) = delete;

		hook_holder_impl(hook_holder_impl&& other) noexcept
		{
			if (this->hooked( ))
			{
				BOOST_ASSERT_MSG(!other.hooked(), "Unable to move active hook");
				Unhook_lazy_( );
			}

			this->hook_data_ = utl::move(other.hook_data_);
			this->hook_params_ = utl::move(other.hook_params_);
		}

	private:
		enum original_type
		{
			member,
			member_const,
			static_fn
		};

		template <original_type Type>
		static auto Recreate_original_type_(LPVOID original_fn)
		{
#define HOOK_UTL_FIX_FN_TYPE(type)\
		        constexpr (Call_cvs == call_conversion::type##__)\
		        {\
                    if constexpr (Type == static_fn)\
                        return (Ret(__##type *)(Args ...))(original_fn);\
                    else if constexpr(Type == member_const)\
		           	    return *(std::add_const_t<Ret(__##type C::**)(Args ...)>)(&original_fn);\
                    else\
                        return *(Ret(__##type C::**)(Args ...))(&original_fn);\
                }

			if HOOK_UTL_FIX_FN_TYPE(thiscall)
			else if HOOK_UTL_FIX_FN_TYPE(cdecl)
			else if HOOK_UTL_FIX_FN_TYPE(stdcall)
			else if HOOK_UTL_FIX_FN_TYPE(vectorcall)
			else if HOOK_UTL_FIX_FN_TYPE(fastcall)

#undef HOOK_UTL_FIX_FN_TYPE
		}

	public:
		Ret call_original(Args ...args) const
			requires(Is_const)
		{
			//Unset_instance_assert_( );
			Unmanaged_call_assert( );
			auto original = Recreate_original_type_<member_const>(original_func__);
			return utl::invoke(original, target_instance__, static_cast<Args>(args)...);
		}

		Ret call_original(Args ...args)
			requires(!Is_const)
		{
			//Unset_instance_assert_( );
			Unmanaged_call_assert( );
			auto original = Recreate_original_type_<is_static ? static_fn : member>(original_func__);
			if constexpr (is_static)
				return utl::invoke(original, static_cast<Args>(args)...);
			else
				return utl::invoke(original, target_instance__, static_cast<Args>(args)...);
		}

		bool hook( ) final
		{
			BOOST_ASSERT_MSG(original_func__ == nullptr, "Method already hooked!");
			Set_instance_assert_( );

			auto& target_func_info = target_func_;
			auto  replace_func_info = method_info::make_member_virtual(cast_hook_callback( ), 1); //index of callback_proxy from hook_callback class

			if (!target_func_info.update( ) || !replace_func_info.update( ))
				return false;

			const auto result = hooks_context_->create_hook(target_func_info.get( ), replace_func_info.get( ));
			if (result != STATUS::OK)
			{
				BOOST_ASSERT("Unable to hook function");
				return false;
			}

#ifdef _DEBUG
			if (original_func__ != nullptr)
			{
				const auto t = result.entry->buffer( );

				BOOST_ASSERT_MSG(t != original_func__, "Duplicate class detected");
				BOOST_ASSERT_MSG(t == original_func__, "Deleted hook recreated");
			}
#endif
			this_instance__ = this;
			original_func__ = result.entry->buffer( );
			return true;
		}

	private:
		bool Unhook_internal(bool force)
		{
			Unset_instance_assert_( );
			Unmanaged_call_assert( );

			const auto ok = hooks_context_->remove_hook(target_func_.get( ), force) == STATUS::OK;
			if (!ok && !force)
			{
				BOOST_ASSERT("Unable to unhook function");
				return false;
			}

			if (ok)
			{
				original_func__ = nullptr;
				this_instance__ = nullptr;
				target_instance__ = nullptr;
				safe__ = { };
				return_value_ = { };
			}

			return ok;
		}

	protected:
		context_shared::shared_type hooks_context_ = context_shared::get_shared( );

	public:
		bool unhook( ) final
		{
			return Unhook_internal(false);
		}

		void unhook_safe( ) final
		{
			safe__.unhook = true;
		}

		bool unhook_unsafe( ) final
		{
			return Unhook_internal(true);
		}

		bool enable( ) final
		{
			Unmanaged_call_assert( );
			if (hooks_context_->enable_hook(target_func_.get( )) == STATUS::OK)
				return true;

			BOOST_ASSERT("Unable to enable hook");
			return false;
		}

		bool disable( ) final
		{
			Unset_instance_assert_( );
			Unmanaged_call_assert( );
			if (hooks_context_->disable_hook(target_func_.get( )) == STATUS::OK)
			{
				safe__.disable = false;
				return true;
			}

			BOOST_ASSERT("Unable to disable hook");
			return false;
		}

		void disable_safe( ) final
		{
			safe__.disable = true;
		}

	private:
		hook_result Get_hook_from_storage( ) const
		{
			Unmanaged_call_assert( );
			return target_func_.updated( )
				   ? hooks_context_->find_hook(target_func_.get( ))
				   : STATUS::ERROR_NOT_CREATED;
		}

	public:
		bool hooked( ) const final
		{
			const auto hook = Get_hook_from_storage( );
			if (hook != STATUS::OK)
			{
				//unhook_cleanup( );
				return false;
			}
			(void)hook;
			Unset_instance_assert_( );
			return true;
		}

		bool enabled( ) const final
		{
			const auto hook = Get_hook_from_storage( );
			if (hook != STATUS::OK)
			{
				//unhook_cleanup( );
				return false;
			}
			Unset_instance_assert_( );
			return hook.entry->enabled;
		}

#if 0
		private:
			static auto Get_finished_future_() -> future<void>
			{
				boost::promise<void> p;
				p.set_value();
				return p.get_future();
			}

			template <class Rep, class Period, class Fn>
			auto Invoke_after(bool& check, const chrono::duration<Rep, Period>& delay, Fn&& fn) -> future<void>
			{
				if (check)
					return Get_finished_future_();

				check = true;

				return boost::async(boost::launch::async, [check = ref(check), delay, fn_callback = utl::forward<Fn>(fn)]
					{
						using clock = chrono::steady_clock;
						auto end = clock::now() + delay;

						while (clock::now() < end)
						{
							if (!check)
								return;
							this_thread::sleep_for(chrono::milliseconds(1));
						}

						fn_callback();
					});
			}

		public:
			template <class Rep, class Period>
			auto unhook_safe(const chrono::duration<Rep, Period>& delay) -> future<void>
			{
				Unset_instance_assert_();
				Unmanaged_call_assert();
				if (!this->hooked())
					return Get_finished_future_();
				return Invoke_after(safe__.unhook, delay, boost::bind(&hook_holder_base::unhook, this));
			}

			template <class Rep, class Period>
			auto disable_safe(const chrono::duration<Rep, Period>& delay) -> future<void>
			{
				Unset_instance_assert_();
				Unmanaged_call_assert();
				if (!this->enabled())
					return Get_finished_future_();
				return Invoke_after(safe__.disable, delay, boost::bind(&hook_holder_base::disable, this));
			}
#endif
	};
}

	template <typename Ret, call_conversion Call_cvs, typename C, bool Is_const, typename ...Args>
	struct hook_holder;

	//todo: fix args for non-class functions

#pragma region call_cvs_2

	namespace detail
	{
		template <typename T, size_t ...I>
		auto _Shift_left_impl(T& tpl, std::index_sequence<I...>)
		{
			//return utl::forward_as_tuple(reinterpret_cast<decltype(get<I + 1>(tpl))>(get<I>(tpl))...);
			return utl::forward_as_tuple(reinterpret_cast<decltype(get<I + 1>(tpl))>(get<I>(tpl)).unhide( )...);
		}

		template <typename ...T>
		auto _Shift_left(utl::tuple<hiddent_type<T>...>&& tpl)
		{
			return _Shift_left_impl(tpl, std::make_index_sequence<sizeof...(T) - 1>( ));
		}
	}

#define CALL_CVS_STUFF_IMPL2(call_cvs)\
    template <typename Ret, typename C, typename ...Args>\
    struct hook_holder<Ret, call_conversion::call_cvs##__, C, false, Args...>:\
      detail::hook_holder_impl<Ret, call_conversion::call_cvs##__, C, false, Args...>\
    {\
        Ret __##call_cvs callback_proxy(hiddent_type<Args> ...args) final\
        {\
            if constexpr (std::is_class_v<C>)\
				return this->callback_proxy_impl(args.unhide()...);\
			else\
			{\
				return apply(&hook_holder::callback_proxy_impl,\
							utl::tuple_cat(utl::tuple(this), detail::_Shift_left(utl::tuple(hiddent_type<void*>(this->cast_hook_callback( )), args...))));\
			}\
        }\
    };\
    template <typename Ret, typename C, typename ...Args>\
    struct hook_holder<Ret, call_conversion::call_cvs##__, C, true, Args...>:\
      detail::hook_holder_impl<Ret, call_conversion::call_cvs##__, C, true, Args...>\
    {\
        Ret __##call_cvs callback_proxy(hiddent_type<Args> ... args) /*const*/ final\
        {\
            if constexpr (std::is_class_v<C>)\
				return this->callback_proxy_impl(args.unhide()...);\
			else\
			{\
				return apply(&hook_holder::callback_proxy_impl,\
							utl::tuple_cat(utl::tuple(this), detail::_Shift_left(utl::tuple(hiddent_type<void*>(this->cast_hook_callback( )), args...))));\
			}\
        }\
    };\
    template <typename Ret, typename C, typename ...Args>\
    auto detect_hook_holder(Ret (__##call_cvs C::*fn)(Args ...))		-> hook_holder<Ret, call_conversion::call_cvs##__, C, false, Args...>\
        { return {}; }\
    template <typename Ret, typename C, typename ...Args>\
    auto detect_hook_holder(Ret (__##call_cvs C::*fn)(Args ...) const)	-> hook_holder<Ret, call_conversion::call_cvs##__, C, true, Args...>\
        { return {}; }\
    template <typename Ret, typename ...Args>\
    auto detect_hook_holder(Ret (__##call_cvs    *fn)(Args ...))		-> hook_holder<Ret, call_conversion::call_cvs##__, void, false, Args...>\
        { return {}; }

	CALL_CVS_STUFF_IMPL2(thiscall)
	CALL_CVS_STUFF_IMPL2(fastcall)
	CALL_CVS_STUFF_IMPL2(stdcall)
	CALL_CVS_STUFF_IMPL2(vectorcall)
	CALL_CVS_STUFF_IMPL2(cdecl)
#undef CALL_CVS_STUFF_IMPL2

#pragma endregion
}
