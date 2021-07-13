#pragma once

#include "cheat/core/service.h"

namespace cheat
{
	class console final: public service_shared<console, service_mode::sync>
	{
	public:
		~console( ) override;
		console( );

		void write(const utl::string_view& str) const;
		void write_time( ) const;
		void write_line(const utl::string_view& str) const;
		void write_char(char c) const;

	protected:
		void Load( ) override;

	private:
		bool  write_redirected__ = false;
		FILE* write__ = nullptr;

		bool console_allocated__ = false;
		HWND console_window__ = nullptr;

		void Wait_for_write_( ) const;
	};
}
