#include "win_user.h"

using namespace utl::winapi::detail;
using namespace utl::winapi;

#pragma region window

void window_destroyer::operator()(HWND ptr) const
{
    DestroyWindow(ptr);
}

auto window::style( ) -> _Setter_native<window_style_flags>
{
    return this->_Init_setter_native(reinterpret_cast<window_style_flags&>(this->style_));
}

auto window::style( ) const -> const window_style_flags&
{
    return reinterpret_cast<const window_style_flags&>(this->style_);
}

auto window::name( ) -> _Setter_string
{
    return this->_Init_setter_native(this->name_);
}

auto window::name( ) const -> std::basic_string_view<default_stable_string::value_type>
{
    return (this->name_);
}

auto window::position( ) -> _Setter_native<position_memders>
{
    return this->_Init_setter_native(this->position_);
}

auto window::position( ) const -> const position_memders&
{
    return this->position_;
}

auto window::size( ) -> _Setter_native<size_memders>
{
    return this->_Init_setter_native(this->size_);
}

auto window::size( ) const -> const size_memders&
{
    return this->size_;
}

bool window::is_created( ) const
{
    void* ptr = this->hwnd_.get( );
    return ptr != nullptr;
}

HWND window::handle( ) const
{
    return this->hwnd_.get( );
}

window::_Is_created_lazy_t window::_Is_created_lazy( ) const
{
    return std::bind_front(&window::is_created, this);
}

#pragma region window_controller

bool window::_Controller_const::show(int show_command) const
{
    BOOST_ASSERT_MSG(owner().is_created(), "Unable to show unregistered window!");
    return ShowWindow(owner( ).handle( ), show_command);
}

bool window::_Controller_const::update( ) const
{
    BOOST_ASSERT_MSG(owner().is_created(), "Unable to update unregistered window!");
    return UpdateWindow(owner( ).handle( ));
}

const window& window::_Controller_const::owner( ) const
{
    return owner_;
}

//-------------

void window::_Controller::create(const window_class& sample) const
{
    auto& wnd = this->owner( );
    BOOST_ASSERT_MSG(!wnd.is_created(), "Unable to modfiy registered window!");
    BOOST_ASSERT_MSG(sample.is_registered(), "Sample class not registered");

    auto& x = wnd.position_.x;
    auto& y = wnd.position_.y;
    BOOST_ASSERT_MSG(x >= 0 && y >= 0, "Incorrect window position!");

    auto& w = wnd.size_.width;
    auto& h = wnd.size_.height;
    BOOST_ASSERT_MSG(w > 0 && h > 0, "Incorrect window size!");

    const auto hwnd = CreateWindow(sample.class_name().data(), wnd.name_.data(), static_cast<std::underlying_type_t<window_style>>(wnd.style_),
                                   x, y, w, h,
                                   nullptr, nullptr, sample.instance(), nullptr);

    wnd.hwnd_ = decltype(wnd.hwnd_)(hwnd);
}

void window::_Controller::destory( ) const
{
    owner( ).hwnd_.reset( );
}

window& window::_Controller::owner( ) const
{
    return const_cast<window&>(_Controller_const::owner( ));
}

#pragma endregion

window::_Controller window::control( )
{
    return _Controller(*this);
}

window::_Controller_const window::control( ) const
{
    return _Controller_const(*this);
}
#pragma endregion

void icon_destroyer::operator()(HICON ptr) const
{
    DestroyIcon(ptr);
}

#pragma region window_class

void window_class_unregister::operator()(WNDCLASSEX* ptr) const
{
    UnregisterClass(ptr->lpszClassName, ptr->hInstance);
}

window_class::window_class( )
{
    memory_ = decltype(memory_)(memory_.get_deleter( ).allocate(1));
    std::memset(memory_.get( ), 0, sizeof(WNDCLASSEX));
    memory_->cbSize = sizeof(WNDCLASSEX);
}

auto window_class::style( ) -> _Setter_native<class_types_flags>
{
    return this->_Init_setter_native(reinterpret_cast<class_types_flags&>(this->memory_->style));
}

auto window_class::style( ) const -> class_types_flags
{
    return (this->memory_->style);
}

auto window_class::wnd_proc( ) -> _Setter_native<WNDPROC>
{
    return this->_Init_setter_native(this->memory_->lpfnWndProc);
}

auto window_class::wnd_proc( ) const -> WNDPROC
{
    return this->memory_->lpfnWndProc;
}

auto window_class::instance( ) -> _Setter_native<HINSTANCE>
{
    return this->_Init_setter_native(this->memory_->hInstance);
}

auto window_class::instance( ) const -> HINSTANCE
{
    return this->memory_->hInstance;
}

auto window_class::icon( ) -> _Setter_native<HICON>
{
    return this->_Init_setter_native(this->memory_->hIcon);
}

auto window_class::icon( ) const -> HICON
{
    return this->memory_->hIcon;
}

auto window_class::cursor( ) -> _Setter_native<HCURSOR>
{
    return this->_Init_setter_native(this->memory_->hCursor);
}

auto window_class::cursor( ) const -> HCURSOR
{
    return this->memory_->hCursor;
}

auto window_class::background( ) -> _Setter_native<HBRUSH>
{
    return this->_Init_setter_native(this->memory_->hbrBackground);
}

auto window_class::background( ) const -> HBRUSH
{
    return this->memory_->hbrBackground;
}

auto window_class::menu_name( ) -> _Setter_string
{
    return this->_Init_setter_string(this->menu_name_, this->memory_->lpszMenuName);
}

auto window_class::menu_name( ) const -> std::basic_string_view<default_stable_string::value_type>
{
    return (this->menu_name_);
}

auto window_class::class_name( ) -> _Setter_string
{
    return this->_Init_setter_string(this->class_name_, this->memory_->lpszClassName);
}

auto window_class::class_name( ) const -> std::basic_string_view<default_stable_string::value_type>
{
    return (this->class_name_);
}

auto window_class::small_icon( ) -> _Setter_native<HICON>
{
    return this->_Init_setter_native(this->memory_->hIconSm);
}

auto window_class::small_icon( ) const -> HICON
{
    return this->memory_->hIconSm;
}

bool window_class::is_registered( ) const
{
    return this->registered_ != nullptr;
}

ATOM window_class::registered_atom( ) const
{
    return this->is_registered( ) ? registered_atom_ : 0;
}

void window_class::register_class( )
{
    BOOST_ASSERT_MSG(!is_registered(), "Class already rgistered!");

    const auto memory_ptr = /*std::addressof*/(this->memory_.get( ));

    registered_atom_ = RegisterClassEx(memory_ptr);
    registered_      = decltype(registered_)(memory_ptr);
}

void window_class::unregister_class( )
{
    this->registered_.reset( );
}

window_class::_Is_registered_lazy_t window_class::_Is_registered_lazy( ) const { return std::bind_front(&window_class::is_registered, this); }

window_class::_Setter_string window_class::_Init_setter_string(default_stable_string& string_stored, default_stable_string::const_pointer& mirror) const
{
    return make_value_setter(this->_Is_registered_lazy( ),
                             value_setter_native_getter(string_stored),
                             _String_to_ptr_binder(string_stored, mirror), string_stored);
}

#pragma endregion
