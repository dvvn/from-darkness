module;

#include <nstd/runtime_assert.h>
#include <string>

module cheat.csgo.interfaces.Direct3DDevice9;
import cheat.csgo.modules;
import nstd.rtlib;
import nstd.mem.signature;

using namespace cheat;
using namespace csgo;

extern IDirect3DDevice9* d3dDevice9_ptr;

IDirect3DDevice9* nstd::one_instance_getter<IDirect3DDevice9*>::_Construct( ) const
{
	auto& infos = nstd::rtlib::all_infos::get( );
	if (!infos.empty( ))
	{
		if (infos.front( ) != infos.current( ))
			return csgo_modules::shaderapidx9->find_signature("A1 ? ? ? ? 50 8B 08 FF 51 0C").plus(1).deref<2>( );
	}
	else
	{
		if (!infos.contains_locker( ))
			infos.set_locker([](auto& rng) {return !rng.empty( ); });
		infos.update(false);
	}

	using namespace nstd::mem;

	const auto block = infos.current( ).mem_block( );
	constexpr std::string_view sig = "1 ? ? 3 ? ? 3 ? ? 7";
	const auto bytes = make_signature(sig.begin( ), sig.end( ), signature_convert( ));
	const auto found = block.find_block(bytes);
	const basic_address hint = found.data( );

	return hint.minus(sizeof(uintptr_t)).deref<1>( );
}
