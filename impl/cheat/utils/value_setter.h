#pragma once

//DEPRECATED !!!

namespace cheat::utl
{
	/*namespace detail
	{
		template <typename _Ty>
		constexpr auto _Value_setter_base_fn( )
		{
			if constexpr (!std::is_class_v<_Ty> || !std::copyable<_Ty>)
				return std::type_identity<optional<_Ty> >{ };
			else
				return std::type_identity<_Ty>{ };
		}

		template <typename _Ty>
		using _Value_setter_base = typename decltype(_Value_setter_base_fn<_Ty>( ))::type;
	}*/

	enum class value_setter_mode
	{
		UNKNOWN=0,
		AUTO = 3,
		DIRECT=1 << 0,
		WRAP=1 << 1,
	};

	namespace detail
	{
		template <typename _Ty, typename _Ret, typename ..._Args>
		concept _Invocable_r = true; //std::is_invocable_r_v<_Ret, _Ty, _Args...>;

		template <typename _Ty, value_setter_mode _Mode>
		class _Value_setter_base;

		template <typename _Ty>
		class _Value_setter_base<_Ty, value_setter_mode::DIRECT>: public _Ty
		{
		public:
			using value_type = _Ty;
			using element_type = _Ty;

			static constexpr value_setter_mode mode = value_setter_mode::DIRECT;

		protected:
			const _Ty& _Get_value( ) const { return *this; }
			_Ty&       _Get_value( ) { return *this; }

			void _Store_value(_Ty&& val) { _Get_value( ) = move(val); }

			static void _Reset_value( )
			{
			}

		public:
			static bool _Contains_value( ) { return true; }
		};

		template <typename _Ty>
		class _Value_setter_base<_Ty, value_setter_mode::WRAP>: public optional<_Ty>
		{
		public:
			using value_type = optional<_Ty>;
			using element_type = _Ty;

			static constexpr value_setter_mode mode = value_setter_mode::WRAP;

		protected:
			const _Ty& _Get_value( ) const { return **this; }
			_Ty&       _Get_value( ) { return **this; }

			void _Store_value(_Ty&& val)
			{
				if (!_Contains_value( ))
					this->emplace(move(val));
				else
					_Get_value( ) = move(val);
			}

			void _Reset_value( ) { this->reset( ); }
		public:
			bool _Contains_value( ) const { return this->has_value( ); }
		};

		template <typename _Ty>
		class _Value_setter_base<_Ty, value_setter_mode::AUTO>:
				public _Value_setter_base<_Ty, (std::is_class_v<_Ty> ? value_setter_mode::DIRECT : value_setter_mode::WRAP)>
		{
		};
	}

	template <std::default_initializable _Ty,
		detail::_Invocable_r<bool> _Locked_fn, detail::_Invocable_r<_Ty> _Getter_fn, detail::_Invocable_r<void, _Ty&&> _Setter_fn,
		value_setter_mode _Mode = value_setter_mode::AUTO>
	class value_setter: public detail::_Value_setter_base<_Ty, _Mode>
	{
		using _Base_type = detail::_Value_setter_base<_Ty, _Mode>;

		static_assert(std::copyable<_Ty> || std::is_invocable_r_v<_Ty&, _Getter_fn>);

		_Base_type&       _Base( ) { return *static_cast<_Base_type*>(this); }
		const _Base_type& _Base( ) const { return *static_cast<const _Base_type*>(this); }

#if 0
        void _Set_value(_Ty&& val)
        {
            _Base_type::_Store_value(move(val));

            if (this->released( ) && this->modified( ))
            {
                this->dont_apply_ = false;
            }
        }

        void _Set_value(const _Ty& val)
        {
            _Ty copy = val;
            this->_Set_value(move(copy));
        }
#endif

		template <typename ..._Args>
		void _Set_value(_Args&&...args)
			requires(std::constructible_from<_Ty, decltype(args)...>)
		{
			_Base_type::_Store_value(_Ty(FWD(args)...));
			if (this->_Released( ) && this->_Modified( ))
				this->dont_apply_ = false;
		}

	public:
		using locked_fn = _Locked_fn;
		using getter_fn = _Getter_fn;
		using setter_fn = _Setter_fn;

		~value_setter( )
		{
			this->_Apply( );
		}

		value_setter(locked_fn&& locked_fn, getter_fn&& getter_fn, setter_fn&& setter_fn) : dont_apply_(false),
																							locked_fn_(move(locked_fn)),
																							getter_fn_(move(getter_fn)),
																							setter_fn_(move(setter_fn))
		{
			if constexpr (!std::copyable<_Ty>)
				this->_Sync( );
		}

		value_setter(const value_setter&) = delete;
		value_setter& operator=(const value_setter&) = delete;
		value_setter(value_setter&&) = default;
		value_setter& operator=(value_setter&&) = default;

#if 0
        value_setter(value_setter&& other) noexcept
        {
            *this = move(other);
        }

        value_setter& operator =(value_setter&& other) noexcept
        {
            MOVE_ASSIGN(_Base());
            MOVE_ASSIGN(dont_apply_);
            MOVE_ASSIGN(locked_fn_);
            MOVE_ASSIGN(getter_fn_);
            MOVE_ASSIGN(setter_fn_);

            other.dont_apply_ = true;

            return *this;
        }
    private:
        getter_fn _Native_getter(_Ty& native_owner) const
        {
            return [&]()-> _Ty&
            {
                return native_owner;
            };
        }

    public:

        template <check_function<locked_fn> _FnL, check_function<checker_fn> _FnC = checker_fn>
        value_setter(_FnL&& locked_func, _Ty& native_owner, _FnC&& checker_func = { }) : _Base_type()
        {
            setter_fn temp_setter = [&native_owner](_Ty&& value)
            {
                native_owner = move(value);
            };
            setter_fn  setter_func;
            checker_fn temp_checker = FWD(checker_func);
            if (!temp_checker)
                setter_func = move(temp_setter);
            else
            {
                setter_func = [checker = move(temp_checker), setter = move(temp_setter)](_Ty&& val)
                {
                    if (!std::invoke(checker, val))
                        throw std::logic_error("Unable to set value!");
                    std::invoke(setter, move(val));
                };
            }

            _Init(locked_fn(FWD(locked_func)), _Native_getter(native_owner), move(setter_func));
        }

        template <check_function<locked_fn> _FnL, check_function<setter_fn> _FnS>
        value_setter(_FnL&& locked_func, _Ty& native_owner, _FnS&& setter_func) : _Base_type()
        {
            _Init(locked_fn(FWD(locked_func)), _Native_getter(native_owner), setter_fn(FWD(setter_func)));
        }

        template <check_function<locked_fn> _FnL, check_function<getter_fn> _FnG, check_function<setter_fn> _FnS>
        value_setter(_FnL&& locked_func, _FnG&& getter_func, _FnS&& setter_func) : _Base_type()
        {
            _Init(locked_fn(FWD(locked_func)), getter_fn(FWD(getter_func)), setter_fn(FWD(setter_func)));
        }
#endif

		//private:
		//    value_setter( ) = default;
		//public:

		value_setter& operator=(_Ty&& val)
		{
			this->_Set_value(move(val));
			return *this;
		}

		template <typename _Ty_other>
		value_setter& operator=(_Ty_other&& val)
			requires(std::constructible_from<_Ty, decltype(val)>)
		{
			this->_Set_value(_Ty(FWD(val)));
			return *this;
		}

	private:
		//friend class _Val_setter_proxy;
		bool _Released( ) const { return this->dont_apply_; }

		bool _Modified( ) const
		{
			if (!this->_Contains_value( ))
				return false;
			if constexpr (std::equality_comparable<_Ty>)
				return this->_Get_value( ) != getter_fn_( );
			else
				return true;
		}

		_Ty _Release( )
		{
			dont_apply_ = true;
			_Ty result;
			if (this->_Contains_value( ))
			{
				result = move(this->_Get_value( ));
				this->_Reset_value( );
			}
			_Ty&& result_rv = static_cast<_Ty&&>(result);
			return result_rv;
		}

		bool _Apply( )
		{
			if (dont_apply_)
				return false;
			if (!this->_Modified( ))
				return false;

			if (locked_fn_( ))
			{
				BOOST_ASSERT("Unable to modify locked setter!");
				throw std::runtime_error("Unable to modify locked setter!");
			}

			setter_fn_(this->_Release( ));
			return true;
		}

		void _Sync( )
		{
			dont_apply_ = false;
			if constexpr (std::copyable<_Ty>)
				this->_Set_value(getter_fn_( ));
			else
				this->_Set_value(move(getter_fn_( )));
		}

#if 0
#define VS_PROXY_FN(ret, name, ...)\
            ret name () __VA_ARGS__{\
                return this->owner( ).name( );\
            }

        class _Setter_proxy_const
        {
        public:
            _Setter_proxy_const(const value_setter& owner) : owner_(owner) {}

            VS_PROXY_FN(bool, released, const)

        protected:
            const value_setter& owner( ) const
            {
                return owner_;
            }

            value_setter& owner( )
            {
                return const_cast<value_setter&>(owner_);
            }

        private:
            const value_setter& owner_;
        };
        class _Setter_proxy_1: public _Setter_proxy_const
        {
        public:
            _Setter_proxy_1(value_setter& owner) : _Setter_proxy_const(owner) {}

            VS_PROXY_FN(_Ty, release)
            //VS_PROXY_FN(void, destroy)
            VS_PROXY_FN(bool, apply)

            _Locked_fn locked_fn( )
            {
                auto&& fn = move(this->owner( ).locked_fn_);
                release( );
                return fn;
            }

            _Getter_fn getter_fn( )
            {
                auto&& fn = move(this->owner( ).getter_fn_);
                release( );
                return fn;
            }

            _Setter_fn setter_fn( )
            {
                auto&& fn = move(this->owner( ).setter_fn_);
                release( );
                return fn;
            }
        };
        class _Setter_proxy_2: public _Setter_proxy_1
        {
        public:
            _Setter_proxy_2(value_setter& owner) : _Setter_proxy_1(owner) {}

            VS_PROXY_FN(void, sync)
        };

#undef VS_PROXY_FN
#endif
	private:
		//template <class _Ty1, class _Locked_fn1, class _Getter_fn1, class _Setter_fn1, value_setter_mode _Mode1>
		class _Val_setter_proxy: value_setter //<_Ty1,_Locked_fn1,_Getter_fn1,_Setter_fn1,_Mode1>
		{
		public:
			_Val_setter_proxy( ) = delete;

			_Val_setter_proxy(const _Val_setter_proxy&) = delete;
			_Val_setter_proxy& operator=(const _Val_setter_proxy&) = delete;
			_Val_setter_proxy(_Val_setter_proxy&&) = delete;
			_Val_setter_proxy& operator=(_Val_setter_proxy&&) = delete;

			// ReSharper disable CppRedundantQualifier

#if 0
            template <typename ..._Args>
            void set_value(_Args&&...args) requires(std::constructible_from<_Ty, decltype(args)...>)
            {
                value_setter::set_value(FWD(args)...);
            }
#endif
			_Ty& /*get_*/ value( ) { return value_setter::_Base_type::_Get_value( ); }
			const _Ty& /*get_*/value( ) const { return value_setter::_Base_type::_Get_value( ); }

			bool released( ) const { return value_setter::_Released( ); }
			bool modified( ) const { return value_setter::_Modified( ); }

			_Ty release( ) { return value_setter::_Release( ); }

			bool apply( ) { return value_setter::_Apply( ); }
			void sync( ) { return value_setter::_Sync( ); }

		private:
			template <typename _Fn>
			_Fn _Move_func(_Fn& fn)
			{
				_Fn&& ret = static_cast<_Fn&&>(fn);
				value_setter::_Release( );
				return ret;
			}

		public:
			_Locked_fn locked_fn( ) { return this->_Move_func(value_setter::locked_fn_); }
			_Locked_fn getter_fn( ) { return this->_Move_func(value_setter::getter_fn_); }
			_Locked_fn setter_fn( ) { return this->_Move_func(value_setter::setter_fn_); }

			// ReSharper restore CppRedundantQualifier
		};

	public:
#if 0
        _Setter_proxy_const get_value_setter( ) const
        {
            return (*this);
        }

        auto get_value_setter( )
        {
            if constexpr (!std::copyable<_Ty>)
                return _Setter_proxy_1(*this);
            else
                return _Setter_proxy_2(*this);
        }
#endif

		auto& get_value_setter( ) { return *(_Val_setter_proxy*)this; }
		auto& get_value_setter( ) const { return *(const _Val_setter_proxy*)this; }

	private:
		class _Dont_apply_helper
		{
		public:
			_Dont_apply_helper(const _Dont_apply_helper&) = delete;
			_Dont_apply_helper& operator=(const _Dont_apply_helper&) = delete;

			_Dont_apply_helper(bool val): value_(val)
			{
			}

			_Dont_apply_helper(_Dont_apply_helper&& other) noexcept
			{
				value_ = other.value_;
				other.value_ = true;
			}

			_Dont_apply_helper& operator =(_Dont_apply_helper&& other) noexcept
			{
				value_ = other.value_;
				other.value_ = true;
				return *this;
			}

			operator bool&( ) { return value_; }
			operator const bool&( ) const { return value_; }

			//bool& operator=(bool val) { return value_ = val; }

		private:
			bool value_;
		};

		_Dont_apply_helper dont_apply_;

		locked_fn locked_fn_;
		getter_fn getter_fn_;
		setter_fn setter_fn_;
	};

	template <value_setter_mode _Mode = value_setter_mode::AUTO, std::default_initializable _Ty,
		detail::_Invocable_r<bool> _Locked_fn, detail::_Invocable_r<_Ty> _Getter_fn, detail::_Invocable_r<void, _Ty&&> _Setter_fn>
	auto make_value_setter(_Locked_fn&& l_fn, _Getter_fn&& g_fn, _Setter_fn s_fn, [[maybe_unused]] const _Ty& ty_hint = { })
	{
		using setter = value_setter<_Ty, std::remove_cvref_t<_Locked_fn>, std::remove_cvref_t<_Getter_fn>, std::remove_cvref_t<_Setter_fn>, _Mode>;

		auto l_fn1 = FWD(l_fn);
		auto g_fn1 = FWD(g_fn);
		auto s_fn1 = FWD(s_fn);

		return setter(move(l_fn1), move(g_fn1), move(s_fn1));
	}

	template <typename _Ty>
	class value_setter_native_getter: protected std::reference_wrapper<_Ty>
	{
	public:
		value_setter_native_getter(_Ty& val) : std::reference_wrapper<_Ty>(val)
		{
		}

		//auto operator( )( ) -> std::conditional_t<std::copyable<_Ty>, _Ty&, _Ty&&> { return this->get( ); }
		_Ty&       operator( )( ) { return this->get( ); }
		const _Ty& operator( )( ) const { return this->get( ); }
	};

	template <typename _Ty>
	class value_setter_native_setter: protected value_setter_native_getter<_Ty>
	{
	public:
		value_setter_native_setter(_Ty& val) : value_setter_native_getter<_Ty>(val)
		{
		}

		void operator( )(_Ty&& other) { this->get( ) = move(other); }
	};

	template <value_setter_mode _Mode = value_setter_mode::AUTO, std::default_initializable _Ty, detail::_Invocable_r<bool> _Locked_fn>
	auto make_value_setter(_Locked_fn&& l_fn, _Ty& ty_native)
	{
		auto getter = value_setter_native_getter<_Ty>(ty_native);
		auto setter = value_setter_native_setter<_Ty>(ty_native);

		return make_value_setter<_Mode>(FWD(l_fn), move(getter), move(setter), ty_native);
	}

	/*template <typename _Ty>
	using setter_native = detail::value_setter<true, _Ty>;

	template <typename _Ty>
	using setter_custom = detail::value_setter<false, _Ty>;*/
}
