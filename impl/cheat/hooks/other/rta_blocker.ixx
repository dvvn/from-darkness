module;

#include <dhooks/includes.h>
#include <string_view>

export module cheat.hooks.other:rta_blocker;
export import dhooks;
export import cheat.csgo.modules;

namespace cheat::hooks::other
{
	//return address blocker

	export template<size_t Num>
		class rta_blocker final :public dhooks::select_hook_holder<char(std::false_type::*)(const char*), Num>
	{
	public:
		bool load(const csgo_modules::game_module_base& mod)
		{
			this->set_target_method(mod->find_signature("55 8B EC 56 8B F1 33 C0 57 8B 7D 08"));
			return this->hook( ) && this->enable( );
		}

	protected:
		void callback([[maybe_unused]] const char* module_name) override
		{
			this->store_return_value(1);
		}
	};
}
