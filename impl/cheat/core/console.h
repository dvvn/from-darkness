#pragma once

#include "cheat/core/service.h"

#include <nstd/runtime_assert_fwd.h>

#include <memory>

// ReSharper disable CppInconsistentNaming
struct HWND__;
using HWND = HWND__*;
// ReSharper restore CppInconsistentNaming

#if 1//defined(_DEBUG) || defined(CHEAT_GUI_TEST) || defined(CHEAT_NETVARS_UPDATING)
#define CHEAT_HAVE_CONSOLE
#endif

namespace cheat
{
	namespace detail
	{
		class string_packer
		{
			void init( );

		public:
			using str = std::string;
			using strv = std::string_view;
			using wstr = std::wstring;
			using wstrv = std::wstring_view;
			using ostr = std::ostringstream;
			using wostr = std::wostringstream;

			string_packer(const char* cstr);
			string_packer(const wchar_t* wcstr);

			string_packer(str&& s);
			string_packer(const strv& s);
			string_packer(wstr&& s);
			string_packer(const wstrv& s);

			string_packer(ostr&& os);
			string_packer(const ostr& os);
			string_packer(wostr&& os);
			string_packer(const wostr& os);

			~string_packer( );

			struct data_type;
			std::unique_ptr<data_type> packed;
		};
	}

	class console final : public service<console>
#ifdef _DEBUG
						, nstd::rt_assert_handler
#endif
	{
	public:
		class cache_type;

		console( );
		~console( ) override;

		void write(detail::string_packer&& str);
		void write_line(detail::string_packer&& str);
		void write(char c);
		void write(wchar_t c) = delete;

	protected:
		load_result load_impl( ) noexcept override;
#ifdef _DEBUG
		void handle(bool expression_result, const char* expression, const char* message, const std::source_location& location) noexcept override;
		void handle(const char* message, const std::source_location& location) noexcept override;
		size_t id( ) const override;
#endif

	private:
		bool allocated_ = false;
		HWND handle_    = nullptr;

		std::unique_ptr<cache_type> cache_;

		FILE* in_  = nullptr;
		FILE* out_ = nullptr;
		FILE* err_ = nullptr;
	};

#ifdef CHEAT_HAVE_CONSOLE

#define CHEAT_CONSOLE_LOG(msg) cheat::console::get_ptr( )->write_line(msg);
#else
#define CHEAT_CONSOLE_LOG(msg) (void)0

#endif
}
