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

	template <typename Ret, call_conversion Call_cvs, typename C/*, bool Is_const*/, typename ...Args>
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
		uintptr_t value;

		hiddent_type( ) = default;

		hiddent_type(void* ptr) : value(reinterpret_cast<uintptr_t>(ptr))
		{
		}

		template <typename T1>
		hiddent_type<T1> change_type( ) const
		{
			hiddent_type<T1> ret;
			ret.value = value;
			return ret;
		}

		decltype(auto) unhide( )
		{
			if constexpr (std::is_pointer_v<T>)
				return reinterpret_cast<T>(value);
			else if constexpr (!std::is_lvalue_reference_v<T>)
				return reinterpret_cast<T&>(value);
			else
				return *reinterpret_cast<std::remove_reference_t<T>*>(value);
		}
	};

	namespace detail
	{
		template <typename Fn>
		LPVOID _Ptr_to_fn(Fn fn)
		{
			const auto ptr = reinterpret_cast<void*&>(fn);
			return ptr;
		}
	}

	template <typename Ret, typename C, typename ...Args>
	LPVOID _Pointer_to_class_method(Ret (__thiscall C::* fn)(Args ...))
	{
		return detail::_Ptr_to_fn(fn);
	}

	template <typename Ret, typename C, typename ...Args>
	LPVOID _Pointer_to_class_method(Ret (__thiscall C::* fn)(Args ...) const)
	{
		return detail::_Ptr_to_fn(fn);
	}

	template <typename Ret, typename C, typename ...Args>
	LPVOID _Pointer_to_class_method(Ret (__fastcall C::* fn)(Args ...))
	{
		return detail::_Ptr_to_fn(fn);
	}

	template <typename Ret, typename C, typename ...Args>
	LPVOID _Pointer_to_class_method(Ret (__fastcall C::* fn)(Args ...) const)
	{
		return detail::_Ptr_to_fn(fn);
	}

	template <typename Ret, typename C, typename ...Args>
	LPVOID _Pointer_to_class_method(Ret (__stdcall C::* fn)(Args ...))
	{
		return detail::_Ptr_to_fn(fn);
	}

	template <typename Ret, typename C, typename ...Args>
	LPVOID _Pointer_to_class_method(Ret (__stdcall C::* fn)(Args ...) const)
	{
		return detail::_Ptr_to_fn(fn);
	}

	template <typename Ret, typename C, typename ...Args>
	LPVOID _Pointer_to_class_method(Ret (__cdecl C::* fn)(Args ...))
	{
		return detail::_Ptr_to_fn(fn);
	}

	template <typename Ret, typename C, typename ...Args>
	LPVOID _Pointer_to_class_method(Ret (__cdecl C::* fn)(Args ...) const)
	{
		return detail::_Ptr_to_fn(fn);
	}

	template <typename Ret, typename C, typename ...Args>
	struct hook_callback<Ret, call_conversion::thiscall__, C, Args...>
	{
		virtual ~hook_callback( ) = default;
		virtual Ret __thiscall callback_proxy(hiddent_type<Args> ...) = 0;
	};
	template <typename Ret, typename C, typename ...Args>
	struct hook_callback<Ret, call_conversion::fastcall__, C, Args...>
	{
		virtual ~hook_callback( ) = default;
		virtual Ret __fastcall callback_proxy(hiddent_type<Args> ...) = 0;
	};
	template <typename Ret, typename C, typename ...Args>
	struct hook_callback<Ret, call_conversion::stdcall__, C, Args...>
	{
		virtual ~hook_callback( ) = default;
		virtual Ret __stdcall callback_proxy(hiddent_type<Args> ...) = 0;
	};
	template <typename Ret, typename C, typename ...Args>
	struct hook_callback<Ret, call_conversion::cdecl__, C, Args...>
	{
		virtual ~hook_callback( ) = default;
		virtual Ret __cdecl callback_proxy(hiddent_type<Args> ...) = 0;
	};

	namespace detail
	{
		inline void _Call_fn_trap([[maybe_unused]] call_conversion original, [[maybe_unused]] call_conversion called)
		{
#ifdef _DEBUG
			[[maybe_unused]] const auto a = _ReturnAddress( );
			[[maybe_unused]] const auto b = _AddressOfReturnAddress( );
			constexpr auto _ = 0;
#endif // _DEBUG
		}

		FORCEINLINE void _Call_fn_trap([[maybe_unused]] call_conversion original)
		{
			_Call_fn_trap(original, original);
		}

		template <typename Fn_as, typename Fn_old, typename ...Args>
		decltype(auto) _Call_fn_as(Fn_old func_ptr, Args&& ...args)
		{
			Fn_as callable;
			reinterpret_cast<void*&>(callable) = reinterpret_cast<void*&>(func_ptr);
			return utl::invoke(callable, utl::forward<Args>(args)...);
		}
	}

	template <typename Ret, typename C, typename ...Args>
	Ret _Call_function(Ret (__thiscall C::* fn)(Args ...), C* instance, std::type_identity_t<Args> ...args)
	{
		detail::_Call_fn_trap(call_conversion::thiscall__, call_conversion::fastcall__);
		using fn_t = Ret(__fastcall*)(C*, void*, Args ...);
		return detail::_Call_fn_as<fn_t>(fn, instance, nullptr, args...);
	}

	template <typename Ret, typename C, typename ...Args>
	Ret _Call_function(Ret (__thiscall C::* fn)(Args ...) const, const C* instance, std::type_identity_t<Args> ...args)
	{
		return _Call_function(const_cast<Ret(__thiscall C::*)(Args ...)>(fn), const_cast<C*>(instance), args...);
	}

	template <typename Ret, typename C, typename ...Args>
	Ret _Call_function(Ret (__fastcall C::* fn)(Args ...), C* instance, std::type_identity_t<Args> ...args)
	{
		detail::_Call_fn_trap(call_conversion::fastcall__);
		using fn_t = Ret(__fastcall*)(C*, void*, Args ...);
		return detail::_Call_fn_as<fn_t>(fn, instance, nullptr, args...);
	}

	template <typename Ret, typename C, typename ...Args>
	Ret _Call_function(Ret (__fastcall C::* fn)(Args ...) const, const C* instance, std::type_identity_t<Args> ...args)
	{
		return _Call_function(const_cast<Ret(__fastcall C::*)(Args ...)>(fn), const_cast<C*>(instance), args...);
	}

	template <typename Ret, typename C, typename ...Args>
	Ret _Call_function(Ret (__stdcall C::* fn)(Args ...), C* instance, std::type_identity_t<Args> ...args)
	{
		detail::_Call_fn_trap(call_conversion::stdcall__);
		using fn_t = Ret(__stdcall*)(void*, Args ...);
		return detail::_Call_fn_as<fn_t>(fn, instance, args...);
	}

	template <typename Ret, typename C, typename ...Args>
	Ret _Call_function(Ret (__stdcall C::* fn)(Args ...) const, const C* instance, std::type_identity_t<Args> ...args)
	{
		return _Call_function(const_cast<Ret(__stdcall C::*)(Args ...)>(fn), const_cast<C*>(instance), args...);
	}

	template <typename Ret, typename C, typename ...Args>
	Ret _Call_function(Ret (__cdecl C::* fn)(Args ...), C* instance, std::type_identity_t<Args> ...args)
	{
		detail::_Call_fn_trap(call_conversion::cdecl__);
		using fn_t = Ret(__cdecl*)(void*, Args ...);
		return detail::_Call_fn_as<fn_t>(fn, instance, args...);
	}

	template <typename Ret, typename C, typename ...Args>
	Ret _Call_function(Ret (__cdecl C::* fn)(Args ...) const, const C* instance, std::type_identity_t<Args> ...args)
	{
		return _Call_function(const_cast<Ret(__cdecl C::*)(Args ...)>(fn), const_cast<C*>(instance), args...);
	}

	namespace detail
	{
		template <typename Fn, typename C, typename ...Args>
		decltype(auto) _Call_virtual_fn(Fn fn, C* instance, size_t index, Args&& ...args)
		{
			auto vtable = _Pointer_to_virtual_class_table(instance);
			reinterpret_cast<void*&>(fn) = vtable[index];
			return _Call_function(fn, instance, utl::forward<Args>(args)...);
		}
	}

	template <typename Ret, typename C, typename ...Args>
	Ret _Call_function(Ret (__thiscall C::* fn)(Args ...), C* instance, size_t index, std::type_identity_t<Args> ...args)
	{
		return detail::_Call_virtual_fn(fn, instance, index, utl::forward<Args>(args)...);
	}

	template <typename Ret, typename C, typename ...Args>
	Ret _Call_function(Ret (__thiscall C::* fn)(Args ...) const, const C* instance, size_t index, std::type_identity_t<Args> ...args)
	{
		return _Call_function(const_cast<Ret(__thiscall C::*)(Args ...)>(fn), const_cast<C*>(instance), index, args...);
	}

	template <typename Ret, typename C, typename ...Args>
	Ret _Call_function(Ret (__fastcall C::* fn)(Args ...), C* instance, size_t index, std::type_identity_t<Args> ...args)
	{
		return detail::_Call_virtual_fn(fn, instance, index, utl::forward<Args>(args)...);
	}

	template <typename Ret, typename C, typename ...Args>
	Ret _Call_function(Ret (__fastcall C::* fn)(Args ...) const, const C* instance, size_t index, std::type_identity_t<Args> ...args)
	{
		return _Call_function(const_cast<Ret(__fastcall C::*)(Args ...)>(fn), const_cast<C*>(instance), index, args...);
	}

	template <typename Ret, typename C, typename ...Args>
	Ret _Call_function(Ret (__stdcall C::* fn)(Args ...), C* instance, size_t index, std::type_identity_t<Args> ...args)
	{
		return detail::_Call_virtual_fn(fn, instance, index, utl::forward<Args>(args)...);
	}

	template <typename Ret, typename C, typename ...Args>
	Ret _Call_function(Ret (__stdcall C::* fn)(Args ...) const, const C* instance, size_t index, std::type_identity_t<Args> ...args)
	{
		return _Call_function(const_cast<Ret(__stdcall C::*)(Args ...)>(fn), const_cast<C*>(instance), index, args...);
	}

	template <typename Ret, typename C, typename ...Args>
	Ret _Call_function(Ret (__cdecl C::* fn)(Args ...), C* instance, size_t index, std::type_identity_t<Args> ...args)
	{
		return detail::_Call_virtual_fn(fn, instance, index, utl::forward<Args>(args)...);
	}

	template <typename Ret, typename C, typename ...Args>
	Ret _Call_function(Ret (__cdecl C::* fn)(Args ...) const, const C* instance, size_t index, std::type_identity_t<Args> ...args)
	{
		return _Call_function(const_cast<Ret(__cdecl C::*)(Args ...)>(fn), const_cast<C*>(instance), index, args...);
	}

	template <typename Ret, typename ...Args>
	Ret _Call_function(Ret (__fastcall* fn)(Args ...), std::type_identity_t<Args> ...args) = delete;

	template <typename Ret, typename ...Args>
	Ret _Call_function(Ret (__stdcall* fn)(Args ...), std::type_identity_t<Args> ...args)
	{
		detail::_Call_fn_trap(call_conversion::stdcall__);
		return utl::invoke(fn, args...);
	}

	template <typename Ret, typename ...Args>
	Ret _Call_function(Ret (__cdecl* fn)(Args ...), std::type_identity_t<Args> ...args)
	{
		detail::_Call_fn_trap(call_conversion::cdecl__);
		return utl::invoke(fn, args...);
	}
#pragma endregion

	namespace detail
	{
		template <typename T>
		concept _Invocable = std::invocable<T>;
		template <typename T>
		concept _Not_invocable = !std::invocable<T>;
	}

	class method_info
	{
	public:
		enum class type:uint8_t
		{
			fn_unknown,
			fn_static,
			fn_member,
			fn_member_virtual,
		};

		using func_type = utl::function<LPVOID( )>;

	protected:
		method_info(type method_type, func_type&& func);
		method_info(type method_type, LPVOID func_ptr);

	public:
		method_info( ) = default;

		type get_type( ) const;
		LPVOID get( ) const;
		bool update( );
		bool updated( ) const;

	private:
		type type__ = type::fn_unknown;
		utl::variant<func_type, LPVOID> storage__;

		template <typename T>
		static constexpr size_t Count_pointers_( )
		{
			return std::is_pointer_v<T> ? Count_pointers_<std::remove_pointer_t<T>>( ) + 1 : 0;
		}

		template <detail::_Not_invocable T>
		static auto Pack_pointer_(T&& ptr)
		{
			using raw_t = std::remove_cvref_t<decltype(ptr)>;

			constexpr bool rvalue = std::is_rvalue_reference_v<decltype(ptr)>;

			// ReSharper disable once CppTooWideScopeInitStatement
			constexpr size_t num_pointers = Count_pointers_<raw_t>( );

			if constexpr (num_pointers == 0)
			{
				BOOST_STATIC_ASSERT_MSG(rvalue == false, "Unable to store rvalue reference!");
				//return [ptr = utl::addressof(ptr)] { return ptr; };
				return utl::addressof(ptr);
			}
			else if constexpr (num_pointers == 2)
			{
				BOOST_STATIC_ASSERT_MSG(rvalue == false, "Unable to store rvalue reference to pointer!");
				return [=] { return *ptr; };
			}
			else if constexpr (num_pointers == 1)
			{
				if constexpr (rvalue)
				{
					BOOST_ASSERT_MSG(ptr != nullptr, "Rvalue pointer must be set!");
					//return [=] { return ptr; };
					return ptr;
				}
				else
				{
					//instance can be null or dynamic, so store pointer to it
					return [ptr2 = utl::addressof(ptr)] { return *ptr2; };
				}
			}
			else
			{
				BOOST_STATIC_ASSERT_MSG(std::_Always_false<raw_t>, __FUNCTION__);
				return nullptr;
			}
		}

		template <detail::_Invocable T>
		static auto Pack_pointer_(T&& fn)
		{
			using inst_type = std::invoke_result_t<T>;
			using raw_t = std::remove_cvref_t<inst_type>;

			constexpr bool rvalue = std::is_rvalue_reference_v<inst_type>;

			// ReSharper disable once CppTooWideScopeInitStatement
			constexpr size_t num_pointers = Count_pointers_<raw_t>( );

			if constexpr (num_pointers == 0)
			{
				BOOST_STATIC_ASSERT_MSG(rvalue == false, "Unable to store rvalue reference!");
				return [ptr_fn = utl::forward<T>(fn)]
				{
					auto& ptr = utl::invoke(ptr_fn);
					return utl::addressof(ptr);
				};
			}
			else if constexpr (num_pointers == 2)
			{
				BOOST_STATIC_ASSERT_MSG(rvalue == false, "Unable to store rvalue reference to pointer!");
				return [ptr_fn = utl::forward<T>(fn)]
				{
					auto instance = utl::invoke(ptr_fn);
					return *instance;
				};
			}
			else if constexpr (num_pointers == 1)
			{
				//return lambda for better debugging
				return [ptr_fn = utl::forward<T>(fn)]
				{
					return utl::invoke(ptr_fn);
				};
			}
			else
			{
				BOOST_STATIC_ASSERT_MSG(std::_Always_false<raw_t>, __FUNCTION__);
				return func_type( );
			}
		}

		template <typename T>
		static auto Pack_value_(T&& val)
		{
			if constexpr (detail::_Invocable<T>)
			{
				return [fn_stored = utl::forward<T>(val)]
				{
					return utl::invoke(fn_stored);
				};
			}
			else
			{
				using raw_t = std::remove_cvref_t<T>;

				if constexpr (std::is_rvalue_reference_v<decltype(val)> || std::is_trivially_copyable_v<raw_t>)
				{
					return val;
				}
				else
				{
					return [value_stored = utl::ref(val)]( )-> const raw_t&
					{
						return value_stored.get( );
					};
				}
			}
		}

		template <typename T>
		static auto Try_invoke_(T&& val)
		{
			if constexpr (detail::_Invocable<T>)
				return utl::invoke(val);
			else
				return utl::forward<T>(val);
		}

	public:
		template <typename T>
		static method_info make_static(T&& func)
		{
			return method_info(type::fn_static, Pack_pointer_(utl::forward<T>(func)));
		}

		template <typename T>
		static method_info make_member(T&& func)
		{
			return method_info(type::fn_member,
							   [packed = Pack_pointer_(utl::forward<T>(func))]
							   {
								   decltype(auto) fn = Try_invoke_(packed);
								   return _Pointer_to_class_method(fn);
							   });
		}

		template <typename T, typename I>
		static method_info make_member_virtual(T&& instance, I&& index)
		{
			return method_info(type::fn_member_virtual,
							   [instance_packed = Pack_pointer_(utl::forward<T>(instance)), idx_packed = Pack_value_(utl::forward<I>(index))]
							   {
								   decltype(auto) obj_instance = Try_invoke_(instance_packed);
								   decltype(auto) fn_index = Try_invoke_(idx_packed);
								   return _Pointer_to_virtual_class_table(obj_instance)[fn_index];
							   });
		}

		/*template <std::invocable Fn>
		static method_info make_custom(bool refresh_result, Fn&& func)
		{
			return method_info(type::fn_unknown, refresh_result, utl::forward<Fn>(func));
		}*/
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

		/*struct info: utl::one_instance<info>
		{
			using holder = utl::atomic<hook_holder_base*>;

			holder last_func = nullptr;
			holder current_func = nullptr;
		};*/
	};

	namespace detail
	{
		template <class Ret>
		class lazy_return_value
		{
			utl::optional<Ret> value__;

		public:
			template <typename T>
			void store_value(T&& val)
				requires(std::is_constructible_v<Ret, decltype(val)>)
			{
				value__.emplace(Ret(utl::forward<T>(val)));
			}

			void reset( )
			{
				value__.reset( );
			}

			bool empty( ) const
			{
				return !value__.has_value( );
			}

			Ret get( )
			{
				if constexpr (std::is_copy_constructible_v<Ret>)
					return *value__;
				else
				{
					Ret ret = static_cast<Ret&&>(*value__);
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

		template <typename Ret, call_conversion Call_cvs, typename C, /*bool Is_const,*/ typename ...Args>
		class hook_holder_impl: public hook_holder_base,
								protected hook_callback<Ret, Call_cvs, C, /*Is_const,*/ Args...>

		{
		public:
			static constexpr bool have_return_value = !std::is_void_v<Ret>;
			static constexpr bool is_static = !std::is_class_v<C>;

		protected:
			//using hook_callback_type = hook_callback<Ret, Call_cvs, C, Is_const, Args...>;

			hook_callback<Ret, Call_cvs, C, /*Is_const,*/ Args...>* cast_hook_callback( )
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

			inline static hook_holder_impl* this_instance__ = nullptr;
			/*std::conditional_t<Is_const, const C*, C*>*/
			C* target_instance__ = nullptr;

			struct
			{
				bool unhook = false;
				bool disable = false;
			} safe__;

		protected:
			virtual void Callback(Args ...) = 0;

			auto Target_instance( ) const
			{
				return target_instance__;
			}

			using return_value_holder = lazy_return_value<Ret>;

			bool call_original_first_ = false;
			return_value_holder return_value_;

			// ReSharper disable once CppNotAllPathsReturnValue
			Ret callback_proxy_impl(std::type_identity_t<Args> ...args)
			{
				Unset_instance_assert_( );

				this_instance__->target_instance__ = is_static ? nullptr : reinterpret_cast<C*>(this->cast_hook_callback( ));
				return this_instance__->callback_proxy_impl_body(utl::forward<Args>(args)...);
			}

		private:
			// ReSharper disable once CppNotAllPathsReturnValue
			Ret callback_proxy_impl_body(std::type_identity_t<Args> ...args)
			{
				Unmanaged_call_assert( );

				if (call_original_first_)
					this->call_original_ex(static_cast<Args>(args)...);
				else
					return_value_.reset( );

				this->Callback(utl::forward<Args>(args)...);

				if (return_value_.empty( ))
					this->call_original_ex(static_cast<Args>(args)...);

				if (safe__.unhook)
					this->unhook( );
				else if (safe__.disable)
					this->disable( );

				if constexpr (have_return_value)
					return return_value_.get( );
			}

		protected:
			//call original and store result to use later
			auto call_original_ex(std::type_identity_t<Args> ...args)
			{
				Unmanaged_call_assert( );
				if constexpr (have_return_value)
				{
					return_value_.store_value(this->call_original(static_cast<Args>(args)...));
					if constexpr (std::is_copy_constructible_v<Ret> && std::is_trivially_destructible_v<Ret>)
						return return_value_.get( );
				}
				else
				{
					this->call_original(static_cast<Args>(args)...);
					return_value_.set_original_called(true);
				}
			}

			method_info target_func_;

			~hook_holder_impl( ) override
			{
				if (this->hooked( ))
				{
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
					BOOST_ASSERT_MSG(!other.hooked( ), "Unable to move active hook");
					Unhook_lazy_( );
				}

				this->hook_data_ = utl::move(other.hook_data_);
				this->hook_params_ = utl::move(other.hook_params_);
			}

		private:
			static auto Recreate_original_type_(LPVOID original_fn)
			{
#define HOOK_UTL_FIX_FN_TYPE(type)\
		        constexpr (Call_cvs == call_conversion::type##__)\
		        {\
                    if constexpr (is_static)\
                    {\
						Ret(__##type *fn)(Args ...);\
						(void*&)fn=original_fn;\
		           		return fn;\
					}\
                    else\
                    {\
						Ret(__##type C::*fn)(Args ...);\
						(void*&)fn=original_fn;\
		           		return fn;\
					}\
                }

				if HOOK_UTL_FIX_FN_TYPE(thiscall)
				else if HOOK_UTL_FIX_FN_TYPE(cdecl)
				else if HOOK_UTL_FIX_FN_TYPE(stdcall)
				else if HOOK_UTL_FIX_FN_TYPE(vectorcall)
				else if HOOK_UTL_FIX_FN_TYPE(fastcall)

#undef HOOK_UTL_FIX_FN_TYPE
			}

		public:
			Ret call_original(std::type_identity_t<Args> ...args)
			{
				//Unset_instance_assert_( );
				Unmanaged_call_assert( );
				auto original = Recreate_original_type_(original_func__);
				if constexpr (is_static)
					return _Call_function(original, (args)...);
				else
					return _Call_function(original, target_instance__, (args)...);
			}

			bool hook( ) final
			{
				BOOST_ASSERT_MSG(original_func__ == nullptr, "Method already hooked!");
				Set_instance_assert_( );

				auto& target_func_info = target_func_;
				auto replace_func_info = method_info::make_member_virtual(cast_hook_callback( ), 1); //index of callback_proxy from hook_callback class

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

					return true;
				}

				return false;
			}

		protected:
			context_shared::shared_type hooks_context_ = context_shared::get_ptr( );

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
		};
	}

	template <typename Ret, call_conversion Call_cvs, typename C,/* bool Is_const,*/ typename ...Args>
	struct hook_holder;

#pragma region call_cvs_2

	namespace detail
	{
		template <typename T, size_t ...I>
		auto _Shift_left_impl(T& tpl, std::index_sequence<I...>)
		{
			return utl::forward_as_tuple(reinterpret_cast<decltype(get<I + 1>(tpl))>(get<I>(tpl)).unhide( )...);
		}

		template <typename ...T>
		auto _Shift_left(utl::tuple<hiddent_type<T>...>&& tpl)
		{
			return _Shift_left_impl(tpl, std::make_index_sequence<sizeof...(T) - 1>( ));
		}
	}

#define CALL_CVS_STUFF_IMPL2(call_cvs)\
    /*template <typename Ret, typename C, typename ...Args>\
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
    };*/\
    template <typename Ret, typename C, typename ...Args>\
    struct hook_holder<Ret, call_conversion::call_cvs##__, C,/* true,*/ Args...>:\
      detail::hook_holder_impl<Ret, call_conversion::call_cvs##__, C, /*true,*/ Args...>\
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
    auto _Detect_hook_holder(Ret (__##call_cvs C::*fn)(Args ...))		-> hook_holder<Ret, call_conversion::call_cvs##__, C, /*false,*/ Args...>\
        { return {}; }\
    template <typename Ret, typename C, typename ...Args>\
    auto _Detect_hook_holder(Ret (__##call_cvs C::*fn)(Args ...) const)	-> hook_holder<Ret, call_conversion::call_cvs##__, C, /*true,*/ Args...>\
        { return {}; }\
    template <typename Ret, typename ...Args>\
    auto _Detect_hook_holder(Ret (__##call_cvs    *fn)(Args ...))		-> hook_holder<Ret, call_conversion::call_cvs##__, void,/* false,*/ Args...>\
        { return {}; }

	CALL_CVS_STUFF_IMPL2(thiscall)
	CALL_CVS_STUFF_IMPL2(fastcall)
	CALL_CVS_STUFF_IMPL2(stdcall)
	CALL_CVS_STUFF_IMPL2(vectorcall)
	CALL_CVS_STUFF_IMPL2(cdecl)
#undef CALL_CVS_STUFF_IMPL2

	template <typename T>
	using _Detect_hook_holder_t = decltype(_Detect_hook_holder(std::declval<T>( )));

#pragma endregion
}
