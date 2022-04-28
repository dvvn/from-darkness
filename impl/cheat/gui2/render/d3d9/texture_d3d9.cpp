module;

#include <nstd/runtime_assert.h>

#include <d3d9.h>

module cheat.gui2.texture.d3d9;

using namespace cheat::gui2;

texture_d3d9::~texture_d3d9( )
{
	if(!texture_)
		return;
	texture_->Release( );
}

texture_d3d9::texture_d3d9(IDirect3DTexture9* const tex)
	:texture_(tex)
{
	runtime_assert(tex != nullptr, "Unable to construct from null pointer!");
}

void texture_d3d9::update( )
{
	runtime_assert("Finish the function!");
}