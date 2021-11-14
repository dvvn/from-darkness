#pragma once

#include "cheat/service/include.h"

// ReSharper disable CppInconsistentNaming
struct HWND__;
using HWND = HWND__*;
// ReSharper restore CppInconsistentNaming

namespace cheat
{
	namespace detail
	{
		struct string_packer
		{
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

	class console_impl final : public service<console_impl>
	{
	public:
		class cache_type;

		console_impl( );
		~console_impl( ) override;

		void write(detail::string_packer&& str);
		void write_line(detail::string_packer&& str);
		void write(char c);
		void write(wchar_t c) = delete;

	protected:
		load_result load_impl( ) noexcept override;
	private:
		bool allocated_ = false;
		HWND handle_    = nullptr;

		std::unique_ptr<cache_type> cache_;

		FILE* in_  = nullptr;
		FILE* out_ = nullptr;
		FILE* err_ = nullptr;
	};

	CHEAT_SERVICE_SHARE(console);

#ifdef CHEAT_HAVE_CONSOLE

#define CHEAT_CONSOLE_LOG(msg) cheat::console::get( )->write_line(msg);
#else
#define CHEAT_CONSOLE_LOG(msg) (void)0

#endif
}
