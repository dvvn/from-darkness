module;

#include <nstd/runtime_assert.h>

#include <memory>

module cheat.gui2.render;

using namespace cheat::gui2;

render::~render( ) = default;

//----

//static render_obj _Render;
//
//void set_render_impl(render_obj && obj) noexcept
//{
//	using std::swap;
//	runtime_assert(obj != nullptr, "Incorrect render object");
//	runtime_assert(_Render == nullptr, "Render object already set");
//	swap(_Render, obj);
//}
//
//auto cheat::gui2::get_render( ) noexcept -> render* const
//{
//	runtime_assert(_Render != nullptr, "Render not set!");
//	return _Render.get( );
//}

