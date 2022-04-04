module;

#include <windows.h>

export module cheat.gui:menu;

export namespace cheat::gui::menu
{
	/*namespace widgets
	{
		class tab_bar_with_pages;
	}*/

	/*struct menu final : console::lifetime_notification<menu>, nstd::one_instance<menu>, widgets::abstract_renderable
	{
		menu( );
		~menu( ) override;



	protected:
		void construct( ) noexcept override;
		bool load( )noexcept override;

	private:
		struct impl;
		std::unique_ptr<impl> impl_;
	};*/

	enum class state :UINT
	{

	};

	bool render( );
	bool toggle(UINT msg, WPARAM wparam);
	bool visible( );
	bool updating( );
}
