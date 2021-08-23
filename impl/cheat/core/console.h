#pragma once

#include "cheat/core/service.h"

#if 1//defined(_DEBUG) || defined(CHEAT_GUI_TEST) || defined(CHEAT_NETVARS_UPDATING)
#define CHEAT_HAVE_CONSOLE
#endif

namespace cheat
{
	namespace detail
	{
		class movable_function_base
		{
		public:
			virtual      ~movable_function_base( ) = default;
			virtual void operator()( ) =0;
		};

		template <typename T>
		class movable_function final: public movable_function_base
		{
		public:
			movable_function(const movable_function&)            = delete;
			movable_function& operator=(const movable_function&) = delete;

			movable_function(T&& obj): obj_(std::move(obj))
			{
			}

			void operator()( ) override
			{
				std::invoke(obj_);
			}

		private:
			T obj_;
		};

		template <typename T>
		movable_function(T&&) -> movable_function<std::remove_cvref_t<T>>;

		struct console_data
		{
			bool allocated = false;
			HWND handle    = nullptr;

			std::mutex lock;

			void write_cache( );

			template <typename T>
			void add_to_cache(T&& fn)
			{
				auto ptr = std::make_unique<movable_function<T>>(std::forward<T>(fn));
				cache_.push_back(std::move(ptr));
			}

		private:
			using cache_obj = std::unique_ptr<movable_function_base>;
			std::list<cache_obj> cache_;
		};
	}

	class console final: public service<console>
					   , nstd::rt_assert_handler
#ifndef CHEAT_HAVE_CONSOLE
	, service_skipped_always
#endif
	{
	public:
		console( );
		~console( ) override;

		void write(std::string&& str);
		void write(const std::string_view& str);
		void write_line(const std::string_view& str);
		void write(char c);

		void write(std::wstring&& str);
		void write(const std::wstring_view& str);
		void write_line(const std::wstring_view& str);
		void write(wchar_t c) = delete;

		void write(std::ostringstream&& str);
		void write(const std::ostringstream& str);
		void write_line(const std::ostringstream& str);

		void write(std::wostringstream&& str);
		void write(const std::wostringstream& str);
		void write_line(const std::wostringstream& str);

	protected:
		bool load_impl( ) override;
		void handle_impl(const nstd::chr_wchr& expression, const nstd::chr_wchr& message, const info_type& info) noexcept override;

	private:
		detail::console_data data_;

		using file_ptr = FILE*;
		file_ptr in_=0, out_=0, err_=0;
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
				return (std::forward<V>(val));

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
