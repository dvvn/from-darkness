#pragma once

namespace std
{
	template <typename T>
	class allocator;

	template <class T, class Alloc>
	class vector;
}

namespace nstd
{
	class chr_wchr
	{
	public:
		constexpr chr_wchr(decltype(nullptr))
		{
			ptr_   = nullptr;
			index_ = static_cast<unsigned char>(-1);
		}

		constexpr chr_wchr(const void*)
		{
			ptr_   = nullptr;
			index_ = static_cast<unsigned char>(-1);
		}

		constexpr chr_wchr(const char* wchr)
		{
			ptr_   = wchr;
			index_ = 0;
		}

		constexpr chr_wchr(const wchar_t* chr)
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
			chr_wchr         file_name;
			chr_wchr         function;
			unsigned __int64 line;
		};

		virtual ~rt_assert_handler( ) = default;

		void handle(bool       result,
					chr_wchr&& expression, chr_wchr&& message,
					chr_wchr&& file_name, chr_wchr&&  function, unsigned __int64 line) noexcept;

	protected:
		virtual void handle_impl(const chr_wchr& expression, const chr_wchr& message, const info_type& info) noexcept = 0;
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
		void handle_impl(const chr_wchr& expression, const chr_wchr& message, const info_type& info) noexcept override;

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
		constexpr chr_wchr expr_or_msg(T1&& msg, const T2* msg2 = nullptr)
		{
			return msg2 == nullptr ? chr_wchr(msg) : chr_wchr(msg2);
		}
	}

	__forceinline constexpr void rt_assert_invoker(bool result, chr_wchr&& expression, chr_wchr&& message, chr_wchr&& file_name, chr_wchr&& function, unsigned __int64 line)
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
									static_cast<chr_wchr&&>(expression),
									static_cast<chr_wchr&&>(message),
									static_cast<chr_wchr&&>(file_name),
									static_cast<chr_wchr&&>(function),
									line);
		}
	}
}

#ifndef _CONCAT
#define NSTD_CONCATX(x, y) x##y
#define NSTD_CONCAT(x, y)  NSTD_CONCATX(x, y)
#else
#define NSTD_CONCAT(x, y) _CONCAT(x, y)
#endif

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
