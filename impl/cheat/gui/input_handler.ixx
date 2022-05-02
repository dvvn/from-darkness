module;

#include <windows.h>

#include <cstdint>

export module cheat.gui.input;

class process_result
{
public:
	enum result : uint8_t
	{
		skipped,
		processed,
		interacted
	};

	process_result(const result val);

	operator bool( ) const noexcept;
	bool touched( ) const noexcept;

private:
	result result_;
};


export namespace cheat::gui
{
	[[nodiscard]]
	process_result process_input(HWND window, UINT message, WPARAM w_param, LPARAM l_param);
}