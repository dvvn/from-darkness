#pragma once

#include "cheat/core/service.h"

namespace cheat
{
	class console final: public service<console>
	{
	public:
		~console( ) override;

		void write(const utl::string_view& str) const;
		void write_time( ) const;
		void write_line(const utl::string_view& str) const;
		void write_char(char c) const;

	protected:
		bool Do_load( ) override;

	private:
		bool write_redirected__ = false;
		FILE* write__ = nullptr;

		bool console_allocated__ = false;
		HWND console_window__ = nullptr;
	};

	bool _Log_to_console(const utl::string_view& str);
}
