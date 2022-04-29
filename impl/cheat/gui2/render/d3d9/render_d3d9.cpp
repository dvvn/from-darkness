module;

#include <nstd/runtime_assert.h>

#include <d3d9.h>

module cheat.gui2.render.d3d9;

using namespace cheat::gui2;

texture_d3d9::~texture_d3d9( )
{
	device_->Release( );
}

texture_d3d9::texture_d3d9(pointer ptr)
	:device_(ptr)
{
}

bool texture_d3d9::begin( )
{
	return SUCCEEDED(device_->BeginScene( ));
}

void texture_d3d9::run(objects_storage<>* const root_storage)
{
	runtime_assert("Finish the function!");
}

bool texture_d3d9::end( )
{
	return SUCCEEDED(device_->EndScene( ));
}

