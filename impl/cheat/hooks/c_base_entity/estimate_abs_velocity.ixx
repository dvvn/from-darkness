module;

export module cheat.hooks.c_base_entity.estimate_abs_velocity;
import cheat.csgo.interfaces.C_BaseEntity;
import dhooks;

//namespace cheat::csgo
//{
//	class C_BaseEntity;
//	class Vector;
//}

namespace cheat::hooks::c_base_entity
{
	export class estimate_abs_velocity final :public dhooks::select_hook_holder<void(csgo::C_BaseEntity::*)(csgo::Vector&)>
	{
	public:
		estimate_abs_velocity( );

	protected:
		void callback(csgo::Vector& vel) override;

	private:
		bool hook( ) override;
		bool enable( ) override;
		bool disable( ) override;
	};
}
