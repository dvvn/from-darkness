#pragma once

namespace std
{
	template <typename T>
	class allocator;

	template <class T, class Alloc>
	class vector;

	template <class El, class Tr>
	class basic_ostream;
}

namespace nstd
{
	class rt_assert_arg_t
	{
	public:
		constexpr rt_assert_arg_t(decltype(nullptr))
		{
			ptr_   = nullptr;
			index_ = static_cast<unsigned char>(-1);
		}

		constexpr rt_assert_arg_t(const void*)
		{
			ptr_   = nullptr;
			index_ = static_cast<unsigned char>(-1);
		}

		constexpr rt_assert_arg_t(const char* wchr)
		{
			ptr_   = wchr;
			index_ = 0;
		}

		constexpr rt_assert_arg_t(const wchar_t* chr)
		{
			ptr_   = chr;
			index_ = 1;
		}

		constexpr unsigned char index( ) const
		{
			return index_;
		}

		constexpr bool empty( ) const
		{
			return index_ == static_cast<unsigned char>(-1);
		}

		template <size_t I>
		constexpr auto get( ) const
		{
			if constexpr (I == 0)
				return static_cast<const char*>(ptr_);
			else if constexpr (I == 1)
				return static_cast<const wchar_t*>(ptr_);
		}

	private:
		const void*   ptr_;
		unsigned char index_;
	};

	class rt_assert_handler
	{
	public:
		friend class rt_assert_handler_ex;

		struct info_type
		{
			rt_assert_arg_t  file_name;
			rt_assert_arg_t  function;
			unsigned __int64 line;
		};

		virtual ~rt_assert_handler( ) = default;

		void handle(bool              result,
					rt_assert_arg_t&& expression, rt_assert_arg_t&& message,
					rt_assert_arg_t&& file_name, rt_assert_arg_t&&  function, unsigned __int64 line) noexcept;

	protected:
		virtual void handle_impl(const rt_assert_arg_t& expression, const rt_assert_arg_t& message, const info_type& info) noexcept = 0;
	};

	class rt_assert_handler_ex final: public rt_assert_handler
	{
	public:
		rt_assert_handler_ex( );
		~rt_assert_handler_ex( ) override;

		using data_type = std::vector<rt_assert_handler*, std::allocator<rt_assert_handler*>>;

		rt_assert_handler_ex(const rt_assert_handler_ex&)            = delete;
		rt_assert_handler_ex& operator=(const rt_assert_handler_ex&) = delete;

		rt_assert_handler_ex(rt_assert_handler_ex&& other) noexcept;
		rt_assert_handler_ex& operator=(rt_assert_handler_ex&& other) noexcept;

		void add(rt_assert_handler* handler);
		void remove(rt_assert_handler* handler);

	protected:
		void handle_impl(const rt_assert_arg_t& expression, const rt_assert_arg_t& message, const info_type& info) noexcept override;

	private:
		void* data_;
	};

	extern rt_assert_handler_ex rt_assert_object;

	namespace detail
	{
		template <class>
		inline constexpr bool detect_msg = false;

		template <class T, size_t S>
		inline constexpr bool detect_msg<T[S]> = true;
		template <class T, size_t S>
		inline constexpr bool detect_msg<T(&)[S]> = true;
		template <class T, size_t S>
		inline constexpr bool detect_msg<const T(&)[S]> = true;

		template <typename T1, typename T2 = void>
		constexpr rt_assert_arg_t expr_or_msg(T1&& msg, const T2* msg2 = nullptr)
		{
			return msg2 == nullptr ? rt_assert_arg_t(msg) : rt_assert_arg_t(msg2);
		}
	}

	__forceinline constexpr void rt_assert_invoker(bool              result,
												   rt_assert_arg_t&& expression,
												   rt_assert_arg_t&& message,
												   rt_assert_arg_t&& file_name,
												   rt_assert_arg_t&& function,
												   unsigned __int64  line)
	{
		// ReSharper disable once CppIfCanBeReplacedByConstexprIf
		// ReSharper disable once CppRedundantBooleanExpressionArgument
		if (__builtin_is_constant_evaluated( ))
		{
			if (!result)
				throw;
		}
		else
		{
			// ReSharper disable once CppUnreachableCode
			rt_assert_object.handle(result,
									static_cast<rt_assert_arg_t&&>(expression),
									static_cast<rt_assert_arg_t&&>(message),
									static_cast<rt_assert_arg_t&&>(file_name),
									static_cast<rt_assert_arg_t&&>(function),
									line);
		}
	}
}

namespace std
{
	template <typename E, typename Tr>
	basic_ostream<E, Tr>& operator<<(basic_ostream<E, Tr>& s, const nstd::rt_assert_arg_t& val)
	{
		switch (val.index( ))
		{
			case 0:
				return s << val.get<0>( );
			case 1:
				return s << val.get<1>( );
			default:
				throw;
		}
	}
}

#define NSTD_WIDE(x) NSTD_CONCAT(L, x)

// ReSharper disable CppInconsistentNaming
#ifdef _DEBUG
#define runtime_assert(_ARG_, ...)\
	nstd::rt_assert_invoker(\
		nstd::detail::detect_msg<decltype(_ARG_)> ? false : !!(_ARG_),\
		nstd::detail::detect_msg<decltype(_ARG_)> ? (void*)nullptr : NSTD_WIDE(#_ARG_),\
		nstd::detail::expr_or_msg(NSTD_WIDE(#_ARG_), ##__VA_ARGS__),\
		NSTD_WIDE(__FILE__), NSTD_WIDE(__FUNCSIG__), __LINE__)
#else
#define runtime_assert(...) (void)0
#endif

// ReSharper restore CppInconsistentNaming
