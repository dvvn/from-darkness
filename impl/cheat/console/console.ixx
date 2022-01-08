module;

#include "includes.h"

export module cheat.console;
export import cheat.service;

namespace cheat
{
	struct string_packer
	{
		using value_type = std::variant<
			std::string
			, std::string_view
			, std::wstring
			, std::wstring_view
			, std::ostringstream
			, std::wostringstream>;

		template<typename T>
			requires(std::constructible_from<value_type, T> && !std::same_as<std::remove_cv_t<T>, std::monostate>)
		string_packer(T&& val) :packed(std::forward<T>(val))
		{
		}

		template<typename Chr>
		string_packer(const Chr* cstr) : string_packer(std::basic_string_view<Chr>(cstr))
		{
		}

		value_type packed;
	};

	class console_cache
	{
	public:
		using value_type = std::function<void( )>;
		console_cache( ) = default;

		void write_all( )
		{
			std::ranges::for_each(cache_, &value_type::operator ());
			cache_.clear( );
		}

		void store(value_type&& obj)
		{
			cache_.emplace_back(std::move(obj));
		}

		void lock( )
		{
			lock_.lock( );
		}

		void unlock( )
		{
			lock_.unlock( );
		}

	private:
		std::recursive_mutex lock_;
		std::vector<value_type> cache_;
	};

	export class console final :public dynamic_service<console>, nstd::rt_assert_handler
	{
	public:
		console( );
		~console( ) override;

	private:
		void write(string_packer&& str);
		void write_line(string_packer&& str);
		void write(char c);
		void write(wchar_t c) = delete;

	public:
		template<bool NewLine = true, typename Fmt, typename ...T>
		void log(Fmt&& fmt, T&& ...args)
		{
#ifdef CHEAT_HAVE_CONSOLE
			if constexpr (sizeof...(T) > 0)
			{
				const auto fmt_fixed = [&]( )->decltype(auto)
				{
#ifdef FMT_VERSION
					using val_t = std::ranges::range_value_t<Fmt>;
					if constexpr (std::is_same_v<val_t, char>)//fmt support compile time only for char today
						return fmt::runtime(fmt);
					else
#endif
						return std::forward<Fmt>(fmt);
				};
				auto str = std::format(fmt_fixed( ), std::forward<T>(args)...);
				log<NewLine>(std::move(str));
			}
			else
			{
				if constexpr (NewLine)
					write_line(std::forward<Fmt>(fmt));
				else
					write(std::forward<Fmt>(fmt));
			}
#endif
		}

#if 0
		template<bool Success = true, typename ...Ex>
		_NODISCARD bool on_service_loaded(const basic_service* srv, Ex&& ...extra)
		{
#ifdef CHEAT_HAVE_CONSOLE
			constexpr bool have_extra = sizeof...(Ex) > 0;

			if constexpr (have_extra)
				cache_.lock( );

			const auto srv_name = srv->name( );
			constexpr std::string_view message = Success ? "loaded" : "not loaded";
			this->log("\"{}\" {}.", srv_name, message);
			if constexpr (have_extra)
			{
				this->log<false>(' ');
				this->log<false>(std::forward<Ex>(extra)...);
				cache_.unlock( );
			}
#endif
			return Success;
		}
#endif

	protected:
		void load_async( ) noexcept override;
		bool load_impl( ) noexcept override;
	private:
		bool allocated_ = false;
		HWND handle_ = nullptr;

		console_cache cache_;

		FILE* in_ = nullptr;
		FILE* out_ = nullptr;
		FILE* err_ = nullptr;

		void handle(bool expression_result, const char* expression, const char* message, const std::source_location& location) noexcept override;
		void handle(const char* message, const std::source_location& location) noexcept override;
		size_t id( ) const override;
	};

	//#ifdef CHEAT_HAVE_CONSOLE
	//#include <format>
	//#define CHEAT_CONSOLE_LOG(msg) cheat::console::get( )->write_line(msg);
	//#else
	//#define CHEAT_CONSOLE_LOG(msg) (void)0
	//#endif
}
