module;

#include <RmlUi/Core/Context.h>

#include <windows.h>

export module cheat.gui.context;

using namespace Rml;

class input_result
{
public:
	enum result : uint8_t
	{
		skipped,
		processed,
		interacted
	};

	input_result(const result val);

	operator bool( ) const noexcept;
	bool touched( ) const noexcept;

private:
	result result_;
};

struct context_info
{
	HWND window = nullptr;
};

export namespace cheat::gui
{
	struct context : NonCopyMoveable
	{
		context( );
		~context( );

		Context* operator->( ) const noexcept;
		const context_info& get_info( ) const noexcept;

		input_result input(HWND window, UINT message, WPARAM w_param, LPARAM l_param);
		void render( );

	private:
		context_info info_;
		Context* ctx_;
		wchar_t first_u16_code_unit_ = 0;
	};

	

}