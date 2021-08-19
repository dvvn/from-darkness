#pragma once

#include "hook.h"

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
	LPVOID _Pointer_to_class_method(Ret (__thiscall C::*fn)(Args ...))
	{
		return detail::_Ptr_to_fn(fn);
	}

	template <typename Ret, typename C, typename ...Args>
	LPVOID _Pointer_to_class_method(Ret (__thiscall C::*fn)(Args ...) const)
	{
		return detail::_Ptr_to_fn(fn);
	}

	template <typename Ret, typename C, typename ...Args>
	LPVOID _Pointer_to_class_method(Ret (__fastcall C::*fn)(Args ...))
	{
		return detail::_Ptr_to_fn(fn);
	}

	template <typename Ret, typename C, typename ...Args>
	LPVOID _Pointer_to_class_method(Ret (__fastcall C::*fn)(Args ...) const)
	{
		return detail::_Ptr_to_fn(fn);
	}

	template <typename Ret, typename C, typename ...Args>
	LPVOID _Pointer_to_class_method(Ret (__stdcall C::*fn)(Args ...))
	{
		return detail::_Ptr_to_fn(fn);
	}

	template <typename Ret, typename C, typename ...Args>
	LPVOID _Pointer_to_class_method(Ret (__stdcall C::*fn)(Args ...) const)
	{
		return detail::_Ptr_to_fn(fn);
	}

	template <typename Ret, typename C, typename ...Args>
	LPVOID _Pointer_to_class_method(Ret (__cdecl C::*fn)(Args ...))
	{
		return detail::_Ptr_to_fn(fn);
	}

	template <typename Ret, typename C, typename ...Args>
	LPVOID _Pointer_to_class_method(Ret (__cdecl C::*fn)(Args ...) const)
	{
		return detail::_Ptr_to_fn(fn);
	}

	template <typename Ret, typename C, typename ...Args>
	struct hook_callback<Ret, call_conversion::thiscall__, C, Args...>
	{
		virtual                ~hook_callback( ) = default;
		virtual Ret __thiscall callback_proxy(hiddent_type<Args> ...) = 0;
	};
	template <typename Ret, typename C, typename ...Args>
	struct hook_callback<Ret, call_conversion::fastcall__, C, Args...>
	{
		virtual                ~hook_callback( ) = default;
		virtual Ret __fastcall callback_proxy(hiddent_type<Args> ...) = 0;
	};
	template <typename Ret, typename C, typename ...Args>
	struct hook_callback<Ret, call_conversion::stdcall__, C, Args...>
	{
		virtual               ~hook_callback( ) = default;
		virtual Ret __stdcall callback_proxy(hiddent_type<Args> ...) = 0;
	};
	template <typename Ret, typename C, typename ...Args>
	struct hook_callback<Ret, call_conversion::cdecl__, C, Args...>
	{
		virtual             ~hook_callback( ) = default;
		virtual Ret __cdecl callback_proxy(hiddent_type<Args> ...) = 0;
	};

	namespace detail
	{
		inline void _Call_fn_trap([[maybe_unused]] call_conversion original, [[maybe_unused]] call_conversion called)
		{
#ifdef _DEBUG
			[[maybe_unused]] const auto a = _ReturnAddress( );
			[[maybe_unused]] const auto b = _AddressOfReturnAddress( );
			constexpr auto              _ = 0;
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
			return std::invoke(callable, std::forward<Args>(args)...);
		}
	}

	template <typename Ret, typename C, typename ...Args>
	Ret _Call_function(Ret (__thiscall C::*fn)(Args ...), C* instance, std::type_identity_t<Args> ...args)
	{
		detail::_Call_fn_trap(call_conversion::thiscall__, call_conversion::fastcall__);
		using fn_t = Ret(__fastcall*)(C*, void*, Args ...);
		return detail::_Call_fn_as<fn_t>(fn, instance, nullptr, args...);
	}

	template <typename Ret, typename C, typename ...Args>
	Ret _Call_function(Ret (__thiscall C::*fn)(Args ...) const, const C* instance, std::type_identity_t<Args> ...args)
	{
		return _Call_function(const_cast<Ret(__thiscall C::*)(Args ...)>(fn), const_cast<C*>(instance), args...);
	}

	template <typename Ret, typename C, typename ...Args>
	Ret _Call_function(Ret (__fastcall C::*fn)(Args ...), C* instance, std::type_identity_t<Args> ...args)
	{
		detail::_Call_fn_trap(call_conversion::fastcall__);
		using fn_t = Ret(__fastcall*)(C*, void*, Args ...);
		return detail::_Call_fn_as<fn_t>(fn, instance, nullptr, args...);
	}

	template <typename Ret, typename C, typename ...Args>
	Ret _Call_function(Ret (__fastcall C::*fn)(Args ...) const, const C* instance, std::type_identity_t<Args> ...args)
	{
		return _Call_function(const_cast<Ret(__fastcall C::*)(Args ...)>(fn), const_cast<C*>(instance), args...);
	}

	template <typename Ret, typename C, typename ...Args>
	Ret _Call_function(Ret (__stdcall C::*fn)(Args ...), C* instance, std::type_identity_t<Args> ...args)
	{
		detail::_Call_fn_trap(call_conversion::stdcall__);
		using fn_t = Ret(__stdcall*)(C*, Args ...);
		return detail::_Call_fn_as<fn_t>(fn, instance, args...);
	}

	template <typename Ret, typename C, typename ...Args>
	Ret _Call_function(Ret (__stdcall C::*fn)(Args ...) const, const C* instance, std::type_identity_t<Args> ...args)
	{
		return _Call_function(const_cast<Ret(__stdcall C::*)(Args ...)>(fn), const_cast<C*>(instance), args...);
	}

	template <typename Ret, typename C, typename ...Args>
	Ret _Call_function(Ret (__cdecl C::*fn)(Args ...), C* instance, std::type_identity_t<Args> ...args)
	{
		detail::_Call_fn_trap(call_conversion::cdecl__);
		using fn_t = Ret(__cdecl*)(C*, Args ...);
		return detail::_Call_fn_as<fn_t>(fn, instance, args...);
	}

	template <typename Ret, typename C, typename ...Args>
	Ret _Call_function(Ret (__cdecl C::*fn)(Args ...) const, const C* instance, std::type_identity_t<Args> ...args)
	{
		return _Call_function(const_cast<Ret(__cdecl C::*)(Args ...)>(fn), const_cast<C*>(instance), args...);
	}

	namespace detail
	{
		template <typename Fn, typename C, typename ...Args>
		decltype(auto) _Call_virtual_fn(Fn fn, C* instance, size_t index, Args&& ...args)
		{
			auto vtable                  = _Pointer_to_virtual_class_table(instance);
			reinterpret_cast<void*&>(fn) = vtable[index];
			return _Call_function(fn, instance, std::forward<Args>(args)...);
		}
	}

	template <typename Ret, typename C, typename ...Args>
	Ret _Call_function(Ret (__thiscall C::*fn)(Args ...), C* instance, size_t index, std::type_identity_t<Args> ...args)
	{
		return detail::_Call_virtual_fn(fn, instance, index, std::forward<Args>(args)...);
	}

	template <typename Ret, typename C, typename ...Args>
	Ret _Call_function(Ret (__thiscall C::*fn)(Args ...) const, const C* instance, size_t index, std::type_identity_t<Args> ...args)
	{
		return _Call_function(const_cast<Ret(__thiscall C::*)(Args ...)>(fn), const_cast<C*>(instance), index, args...);
	}

	template <typename Ret, typename C, typename ...Args>
	Ret _Call_function(Ret (__fastcall C::*fn)(Args ...), C* instance, size_t index, std::type_identity_t<Args> ...args)
	{
		return detail::_Call_virtual_fn(fn, instance, index, std::forward<Args>(args)...);
	}

	template <typename Ret, typename C, typename ...Args>
	Ret _Call_function(Ret (__fastcall C::*fn)(Args ...) const, const C* instance, size_t index, std::type_identity_t<Args> ...args)
	{
		return _Call_function(const_cast<Ret(__fastcall C::*)(Args ...)>(fn), const_cast<C*>(instance), index, args...);
	}

	template <typename Ret, typename C, typename ...Args>
	Ret _Call_function(Ret (__stdcall C::*fn)(Args ...), C* instance, size_t index, std::type_identity_t<Args> ...args)
	{
		return detail::_Call_virtual_fn(fn, instance, index, std::forward<Args>(args)...);
	}

	template <typename Ret, typename C, typename ...Args>
	Ret _Call_function(Ret (__stdcall C::*fn)(Args ...) const, const C* instance, size_t index, std::type_identity_t<Args> ...args)
	{
		return _Call_function(const_cast<Ret(__stdcall C::*)(Args ...)>(fn), const_cast<C*>(instance), index, args...);
	}

	template <typename Ret, typename C, typename ...Args>
	Ret _Call_function(Ret (__cdecl C::*fn)(Args ...), C* instance, size_t index, std::type_identity_t<Args> ...args)
	{
		return detail::_Call_virtual_fn(fn, instance, index, std::forward<Args>(args)...);
	}

	template <typename Ret, typename C, typename ...Args>
	Ret _Call_function(Ret (__cdecl C::*fn)(Args ...) const, const C* instance, size_t index, std::type_identity_t<Args> ...args)
	{
		return _Call_function(const_cast<Ret(__cdecl C::*)(Args ...)>(fn), const_cast<C*>(instance), index, args...);
	}

	template <typename Ret, typename ...Args>
	Ret _Call_function(Ret (__fastcall*fn)(Args ...), std::type_identity_t<Args> ...args) = delete;

	template <typename Ret, typename ...Args>
	Ret _Call_function(Ret (__stdcall*fn)(Args ...), std::type_identity_t<Args> ...args)
	{
		detail::_Call_fn_trap(call_conversion::stdcall__);
		return std::invoke(fn, args...);
	}

	template <typename Ret, typename ...Args>
	Ret _Call_function(Ret (__cdecl*fn)(Args ...), std::type_identity_t<Args> ...args)
	{
		detail::_Call_fn_trap(call_conversion::cdecl__);
		return std::invoke(fn, args...);
	}
#pragma endregion

	class hook_holder_base
	{
	protected:
		virtual ~hook_holder_base( ) = default;

	public:
		virtual bool hook( ) = 0;
		virtual bool unhook( ) = 0;
		virtual void unhook_after_call( ) = 0;

		virtual bool enable( ) = 0;
		virtual bool disable( ) = 0;
		virtual void disable_after_call( ) = 0;

		virtual bool hooked( ) const = 0;
		virtual bool enabled( ) const = 0;
	};

	namespace detail
	{
#define CHEAT_HOOKS_CALL_CVS_HELPER(_MACRO_)\
		_MACRO_(thiscall)\
		_MACRO_(cdecl)\
		_MACRO_(stdcall)\
		_MACRO_(vectorcall)\
		_MACRO_(fastcall)

		template <typename Ret, call_conversion Call_cvs, typename ...Args2>
		struct hook_callback_proxy;

		template <typename Ret, call_conversion Call_cvs, typename ...Args2>
		struct original_function;

#define CHEAT_HOOK_ORIGINAL_FN(_CALL_CVS_)\
		template <typename Ret, typename C, typename ...Args>\
		struct original_function<Ret, call_conversion::_CALL_CVS_##__, C, Args...>\
		{\
			Ret (__##_CALL_CVS_ C::*original_fn)(Args ...) = nullptr;\
			C* object_instance;\
		\
			Ret call_original(Args ...args)\
			{\
				return _Call_function(original_fn, object_instance, args...);\
			}\
		};\
		\
		template <typename Ret, typename ...Args>\
		struct original_function<Ret, call_conversion::_CALL_CVS_##__,void, Args...>\
		{\
			Ret (__##_CALL_CVS_ *original_fn)(Args ...) = nullptr;\
		\
			Ret call_original(Args ...args)\
			{\
				return _Call_function(original_fn, args...);\
			}\
		};

		CHEAT_HOOKS_CALL_CVS_HELPER(CHEAT_HOOK_ORIGINAL_FN)

		template <typename Ret, typename Arg1, typename ...Args>
		struct hook_callback
		{
			virtual      ~hook_callback( ) = default;
			virtual void callback(Args ...) = 0;
		};

		template <class Ret>
		class lazy_return_value
		{
			std::optional<Ret> value__;

		public:
			template <typename T>
			void store_value(T&& val)
				requires(std::is_constructible_v<Ret, decltype(val)>)
			{
				value__.emplace(Ret(std::forward<T>(val)));
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
			void set_original_called(bool called = true)
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

			void get( ) { (void)this; }
		};

		struct context_shared
		{
			std::unique_ptr<context_base> ctx;
			std::mutex                    lock;
		};

		struct hook_holder_data
		{
			std::shared_ptr<context_shared> context;

			void init( );

			struct
			{
				bool unhook  = false;
				bool disable = false;
				//-
			} after_call;
		};

		class method_info
		{
		protected:
			virtual ~method_info( ) = default;

			virtual utl::address get_target_method_impl( ) const = 0;

		private:
			utl::address target_method_;

		public:
			utl::address get_target_method( )
			{
				if (target_method_ == nullptr)
					target_method_ = this->get_target_method_impl( );

				return target_method_;
			}

			utl::address get_target_method( ) const
			{
				runtime_assert(target_method_ != nullptr);
				return target_method_;
			}

			virtual utl::address get_replace_method( ) = 0;
		};

		template <typename Ret, call_conversion Call_cvs, typename Arg1, typename ...Args>
		struct hook_holder_impl: hook_holder_base,
								 method_info,
								 original_function<Ret, Call_cvs, Arg1, Args...>,
								 hook_callback<Ret, Arg1, Args...>

		{
		private:
			hook_holder_data data_;

		protected:
			static hook_holder_impl*& instance( )
			{
				static hook_holder_impl* obj;
				return obj;
			}

			lazy_return_value<Ret> return_value_;

		public:
			~hook_holder_impl( ) override
			{
				if (data_.context == nullptr)
					return;

				this->unhook( );
			}

			bool hook( ) final
			{
				if (data_.context == nullptr)
					data_.init( );

				const auto lock = std::scoped_lock(data_.context->lock);
				auto&      ctx  = *data_.context->ctx;

				// ReSharper disable once CppInconsistentNaming
				auto& _Instance = this->instance( );

				runtime_assert(this->original_fn == nullptr, "Method already hooked!");
				runtime_assert(_Instance == nullptr || _Instance == this, "Class duplicate detected!");

				auto target_func  = this->get_target_method( ).ptr<void>( );
				auto replace_func = this->get_replace_method( ).ptr<void>( );

				const auto result = ctx.create_hook(target_func, replace_func);
				if (result.status != hook_status::OK)
				{
					runtime_assert("Unable to hook function");
					return false;
				}

				_Instance                                      = this;
				reinterpret_cast<uint8_t*&>(this->original_fn) = result.entry->buffer( );
				return true;
			}

			bool unhook( ) final
			{
				const auto [context, _] = std::move(data_);

				const auto lock = std::scoped_lock(context->lock);
				auto&      ctx  = *context->ctx;

				auto       target_func = this->get_target_method( ).ptr<void>( );
				const auto ok          = ctx.remove_hook(target_func, true) == hook_status::OK;
				/*if (!ok && !force)
				{
					runtime_assert("Unable to unhook function");
					return false;
				}*/

				return ok;
			}

			void unhook_after_call( ) final
			{
				data_.after_call.unhook = true;
			}

			bool enable( ) final
			{
				const auto lock = std::scoped_lock(data_.context->lock);
				auto&      ctx  = *data_.context->ctx;

				auto target_func = this->get_target_method( ).ptr<void>( );

				return (ctx.enable_hook(target_func) == hook_status::OK);
			}

			bool disable( ) final
			{
				const auto lock = std::scoped_lock(data_.context->lock);
				auto&      ctx  = *data_.context->ctx;

				auto target_func = this->get_target_method( ).ptr<void>( );

				const auto ret = (ctx.disable_hook(target_func) == hook_status::OK);

				data_.after_call.disable = false;
				return ret;
			}

			void disable_after_call( ) final
			{
				data_.after_call.disable = true;
			}

			bool hooked( ) const final
			{
				if (data_.context == nullptr)
					return false;

				const auto lock = std::scoped_lock(data_.context->lock);
				auto&      ctx  = *data_.context->ctx;

				auto target_func = this->get_target_method( ).ptr<void>( );
				return ctx.find_hook(target_func).status == hook_status::OK;
			}

			bool enabled( ) const final
			{
				if (data_.context == nullptr)
					return false;

				const auto lock = std::scoped_lock(data_.context->lock);
				auto&      ctx  = *data_.context->ctx;

				auto target_func = this->get_target_method( ).ptr<void>( );
				auto hook        = ctx.find_hook(target_func);
				if (hook.status != hook_status::OK)
					return false;
				return hook.entry->enabled;
			}

			//----

			Ret call_original_ex(Args ...args)
			{
				if constexpr (!std::is_void_v<Ret>)
				{
					auto ret = this->call_original(args...);
					return_value_.store_value(ret);
					return ret;
				}
				else
				{
					this->call_original(args...);
					return_value_.set_original_called( );
				}
			}

			Ret callback_impl(Args ...args)
			{
				return_value_.reset( );
				this->callback(args...);

				if (return_value_.empty( ))
					this->call_original_ex(args...);

				if (data_.after_call.unhook)
					this->unhook( );
				else if (data_.after_call.disable)
					this->disable( );

				return return_value_.get( );
			}
		};
	}

	template <typename Ret, call_conversion Call_cvs, typename Arg1, typename ...Args>
	struct hook_holder;

	namespace detail
	{
		template <typename T, size_t ...I>
		auto _Shift_left_impl(T& tpl, std::index_sequence<I...>)
		{
			return std::forward_as_tuple
					(
					 reinterpret_cast<std::tuple_element_t<I + 1, T>&>(std::get<I>(tpl))
					.unhide( )...
					);
		}

		template <typename ...T>
		auto _Shift_left(std::tuple<hiddent_type<T>...>&& tpl)
		{
			return _Shift_left_impl(tpl, std::make_index_sequence<sizeof...(T) - 1>( ));
		}
	}

#define CHEAT_HOOK_HOLDER_IMPL(_CALL_CVS_)\
	template <typename Ret, typename Arg1, typename ...Args>\
	struct hook_holder<Ret,  call_conversion::_CALL_CVS_##__, Arg1, Args...>:\
						detail::hook_holder_impl<Ret,  call_conversion::_CALL_CVS_##__, Arg1, Args...>\
	{\
	private:\
		Ret __##_CALL_CVS_ callback_proxy(hiddent_type<Args> ...args)\
		{\
			auto _Instance = this->instance();\
			if constexpr (std::is_class_v<Arg1>)\
			{\
				_Instance->object_instance = (Arg1*)this;\
				return std::invoke(\
								   &hook_holder::callback_impl,\
								   _Instance, args.unhide( )...\
								  );\
			}\
			else\
			{\
				return std::apply(\
								  &hook_holder::callback_impl,\
								  std::tuple_cat(std::tuple(_Instance), detail::_Shift_left(std::tuple((this->get_replace_method_holder( )), args...)))\
								 );\
			}\
		}\
	public:\
		utl::address get_replace_method( ) final\
		{\
			return _Pointer_to_class_method(&hook_holder::callback_proxy);\
		}\
\
	private:\
		hiddent_type<LPVOID> get_replace_method_holder( )\
		{\
			return this;\
		}\
	};

	CHEAT_HOOKS_CALL_CVS_HELPER(CHEAT_HOOK_HOLDER_IMPL)

#define CHEAT_HOOK_HOLDER_DETECTOR(_CALL_CVS_)\
	template <typename Ret, typename C, typename ...Args>\
    auto _Detect_hook_holder(Ret (__##_CALL_CVS_ C::*fn)(Args ...))		-> hook_holder<Ret, call_conversion::_CALL_CVS_##__, C, /*false,*/ Args...>\
        { return {}; }\
    template <typename Ret, typename C, typename ...Args>\
    auto _Detect_hook_holder(Ret (__##_CALL_CVS_ C::*fn)(Args ...) const)	-> hook_holder<Ret, call_conversion::_CALL_CVS_##__, C, /*true,*/ Args...>\
        { return {}; }\
    template <typename Ret, typename ...Args>\
    auto _Detect_hook_holder(Ret (__##_CALL_CVS_    *fn)(Args ...))		-> hook_holder<Ret, call_conversion::_CALL_CVS_##__, void,/* false,*/ Args...>\
        { return {}; }

	CHEAT_HOOKS_CALL_CVS_HELPER(CHEAT_HOOK_HOLDER_DETECTOR)

	template <typename T>
	using _Detect_hook_holder_t = decltype(_Detect_hook_holder(std::declval<T>( )));
}
