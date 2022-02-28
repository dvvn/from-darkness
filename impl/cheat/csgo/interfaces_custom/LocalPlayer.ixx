module;

export module cheat.csgo.interfaces.LocalPlayer;
export import cheat.csgo.interfaces.C_CSPlayer;
import nstd.one_instance;

export namespace cheat::csgo
{
	using LocalPlayer = nstd::one_instance<C_CSPlayer**>;
}