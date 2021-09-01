#pragma once

#include "cheat/core/service.h"

#include "nstd/runtime assert.h"

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
	class console final: public service<console>
					   , nstd::rt_assert_handler
#ifndef CHEAT_HAVE_CONSOLE
					   , service_always_skipped
#endif
	{
	public:
		class cache_type;

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
		load_result load_impl( ) override;
		void        handle_impl(const nstd::rt_assert_arg_t& expression, const nstd::rt_assert_arg_t& message, const info_type& info) noexcept override;

	private:
		bool allocated_ = false;
		HWND handle_    = nullptr;

		std::unique_ptr<cache_type> cache_;

		FILE* in_  = 0;
		FILE* out_ = 0;
		FILE* err_ = 0;
	};

#ifdef CHEAT_HAVE_CONSOLE

#define CHEAT_CONSOLE_LOG(msg) cheat::console::get_ptr( )->write_line(msg);
#else
#define CHEAT_CONSOLE_LOG(msg) (void)0

#endif
}
