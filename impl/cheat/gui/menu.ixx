module;

#include "widgets/absrtact_renderable.h"
#include <windows.h>
#include <memory>

export module cheat.gui.menu;
export import cheat.gui.context;

export namespace cheat::gui
{
	/*namespace widgets
	{
		class tab_bar_with_pages;
	}*/

	struct menu final : dynamic_service<menu>, widgets::abstract_renderable
	{
		menu( );
		~menu( ) override;

		bool render( ) override;
		bool toggle(UINT msg, WPARAM wparam);
		bool visible( )const;
		bool updating( )const;

	protected:
		bool load_impl( )noexcept override;

	private:
		struct impl;
		std::unique_ptr<impl> impl_;
	};

	//CHEAT_SERVICE_SHARE(menu);
}
