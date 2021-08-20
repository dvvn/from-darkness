#pragma once

#include "cheat/core/service.h"

#if 1//defined(_DEBUG) || defined(CHEAT_GUI_TEST) || defined(CHEAT_NETVARS_UPDATING)
#define CHEAT_HAVE_CONSOLE
#endif

namespace cheat
{
	namespace detail
	{
		struct console_data
		{
			bool               write_redirected__ = false;
			FILE*              write__            = nullptr;
			std::optional<int> original_mode;

			bool console_allocated__ = false;
			HWND console_window__    = nullptr;

			using cache_obj = std::function<void(FILE*)>;

			std::mutex lock__;
			std::queue<cache_obj,
					   std::list<cache_obj>> cache__;
			void                             write_cache( );
		};
	}

	class console final: public service<console>
					   , detail::console_data
#ifndef CHEAT_HAVE_CONSOLE
	, service_skipped_always
#endif

	{
	public:
		~console( ) override;

		void write(std::string&& str);
		void write(const std::string_view& str);
		void write_line(const std::string_view& str);
		void write(char c);

		void write_line(const std::wstring_view& str);

	protected:
		bool load_impl( ) override;
	};

#ifdef CHEAT_HAVE_CONSOLE

	namespace detail
	{
		template <typename V, typename ...T>
		constexpr decltype(auto) format_proxy(V&& val, T&& ...values)
		{
			if constexpr (sizeof...(T) >= 1)
				return std::format(std::forward<V>(val), std::forward<T>(values)...);
			else
				return nstd::as_string(std::forward<V>(val));

			/*else if constexpr (std::destructible<std::formatter<std::remove_cvref_t<V>>>)
				return std::format("{}", std::forward<V>(val));*/
		}

		template <typename ...T>
		void console_log(T&&...values)
		{
			console::get_ptr( )->write_line(format_proxy(std::forward<T>(values)...));
		}
	}

#define CHEAT_CONSOLE_LOG(...) cheat::detail::console_log(__VA_ARGS__);
#else
#define CHEAT_CONSOLE_LOG(...) (void)0
#endif
}
