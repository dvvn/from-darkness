module;


export module cheat.hooks.vgui_surface.lock_cursor;
import cheat.csgo.interfaces.VguiSurface;
import dhooks;

namespace cheat::hooks::vgui_surface
{
	export class lock_cursor final :public dhooks::select_hook_holder<void(csgo::ISurface::*)()>
	{
	public:
		lock_cursor( );

	protected:
		void callback( ) override;

	private:
		bool hook( ) override;
		bool enable( ) override;
		bool disable( ) override;
	};
}
