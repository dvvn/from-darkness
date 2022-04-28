module;

#include <nstd/runtime_assert.h>

module cheat.gui2.texture;

using namespace cheat::gui2;

texture_renderer::~texture_renderer( ) = default;

auto texture_renderer::get_texture( ) const noexcept -> pointer
{
	return texture_.get( );
}

auto texture_renderer::set_texture(value_type && tex) noexcept -> pointer
{
	runtime_assert(tex != nullptr, "Unable to set null texture!");
	using std::swap;
	swap(texture_, tex);
	return texture_.get( );
}