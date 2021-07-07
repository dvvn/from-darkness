#pragma once

#include "utl/bitflag.h"
#include "utl/value_setter.h"

#include <functional>
#include <windows.h>

namespace utl::winapi
{
    namespace detail
    {
        template <class _Elem, class _Traits = std::char_traits<_Elem>, class _Alloc = std::allocator<_Elem>>
        class _Stable_string: public std::basic_string<_Elem, _Traits, _Alloc>
        {
            using _Base = std::basic_string<_Elem, _Traits, _Alloc>;

            void _Force_allocate(const std::basic_string_view<_Elem>& str)
            {
                BOOST_ASSERT_MSG(!str.empty(), "Unable to initlize from empty data!");

                auto wish_size = std::max(this->capacity( ), str.size( )) + 1;

                this->reserve(wish_size);
                this->assign(str);
            }

        public:
            /*
             //this added because of next shit

             std::string a="text"
             void *ptr=a.data();//or c_str();
             std::string b=std::move(a);
             void *ptr2=b.data();

             bool result = ptr==ptr2;//this is false sometimes, because of short string optimization
             */

            ///----

            template <typename _Traits_other>
            _Stable_string(const std::basic_string_view<_Elem, _Traits_other>& str)
            {
                _Force_allocate((str));
            }

            template <typename _Traits_other, std::same_as<_Alloc> _Alloc_other>
            _Stable_string(std::basic_string<_Elem, _Traits_other, _Alloc_other>&& str)
            {
                if (str.capacity( ) > this->capacity( ))
                    this->assign(std::move(str));
                else
                    this->_Force_allocate(std::basic_string_view<_Elem, _Traits_other>(str));
            }

            /*template <typename _Traits_other, std::_Not_same_as<_Alloc> _Alloc_other>
            _Stable_string(std::basic_string<_Elem, _Traits_other, _Alloc_other>&& str)
            {
                this->_Force_allocate(utl::to_string_view(str));
            }*/

            _Stable_string( )
            {
                this->reserve(this->capacity( ) + 1);
            }
        };
    }

#ifdef UNICODE
    using default_stable_string = detail::_Stable_string<WCHAR>;
#else
     using default_stable_string = detail::_Stable_string<TCHAR>;
#endif

    enum class window_style :DWORD
    {
        /*
 * Window Styles
 */
#
        OVERLAPPED = WS_OVERLAPPED,
        POPUP = WS_POPUP,
        CHILD = WS_CHILD,
        MINIMIZE = WS_MINIMIZE,
        VISIBLE = WS_VISIBLE,
        DISABLED = WS_DISABLED,
        CLIPSIBLINGS = WS_CLIPSIBLINGS,
        CLIPCHILDREN = WS_CLIPCHILDREN,
        MAXIMIZE = WS_MAXIMIZE,
        CAPTION = WS_CAPTION,
        BORDER = WS_BORDER,
        DLGFRAME = WS_DLGFRAME,
        VSCROLL = WS_VSCROLL,
        HSCROLL = WS_HSCROLL,
        SYSMENU = WS_SYSMENU,
        THICKFRAME = WS_THICKFRAME,
        GROUP = WS_GROUP,
        TABSTOP = WS_TABSTOP,

        MINIMIZEBOX = WS_MINIMIZEBOX,
        MAXIMIZEBOX = WS_MAXIMIZEBOX,

        TILED = WS_TILED,
        ICONIC = WS_ICONIC,
        SIZEBOX = WS_SIZEBOX,
        TILEDWINDOW = WS_TILEDWINDOW,

        /*
         * Common Window Styles
         */
        OVERLAPPEDWINDOW = WS_OVERLAPPEDWINDOW,

        POPUPWINDOW = WS_POPUPWINDOW,

        CHILDWINDOW = WS_CHILDWINDOW,

        /*
         * Extended Window Styles
         */
        EX_DLGMODALFRAME = WS_EX_DLGMODALFRAME,
        EX_NOPARENTNOTIFY = WS_EX_NOPARENTNOTIFY,
        EX_TOPMOST = WS_EX_TOPMOST,
        EX_ACCEPTFILES = WS_EX_ACCEPTFILES,
        EX_TRANSPARENT = WS_EX_TRANSPARENT,
#if(WINVER >= 0x0400)
        EX_MDICHILD = WS_EX_MDICHILD,
        EX_TOOLWINDOW = WS_EX_TOOLWINDOW,
        EX_WINDOWEDGE = WS_EX_WINDOWEDGE,
        EX_CLIENTEDGE = WS_EX_CLIENTEDGE,
        EX_CONTEXTHELP = WS_EX_CONTEXTHELP,

#endif /* WINVER >= 0x0400 */
#if(WINVER >= 0x0400)

        EX_RIGHT = WS_EX_RIGHT,
        EX_LEFT = WS_EX_LEFT,
        EX_RTLREADING = WS_EX_RTLREADING,
        EX_LTRREADING = WS_EX_LTRREADING,
        EX_LEFTSCROLLBAR = WS_EX_LEFTSCROLLBAR,
        EX_RIGHTSCROLLBAR = WS_EX_RIGHTSCROLLBAR,

        EX_CONTROLPARENT = WS_EX_CONTROLPARENT,
        EX_STATICEDGE = WS_EX_STATICEDGE,
        EX_APPWINDOW = WS_EX_APPWINDOW,

        EX_OVERLAPPEDWINDOW = WS_EX_OVERLAPPEDWINDOW,
        EX_PALETTEWINDOW = WS_EX_PALETTEWINDOW,

#endif /* WINVER >= 0x0400 */

#if(_WIN32_WINNT >= 0x0500)
        EX_LAYERED = WS_EX_LAYERED,

#endif /* _WIN32_WINNT >= 0x0500 */

#if(WINVER >= 0x0500)
        EX_NOINHERITLAYOUT = WS_EX_NOINHERITLAYOUT,
#endif /* WINVER >= 0x0500 */

#if(WINVER >= 0x0602)
        EX_NOREDIRECTIONBITMAP = WS_EX_NOREDIRECTIONBITMAP,
#endif /* WINVER >= 0x0602 */

#if(WINVER >= 0x0500)
        EX_LAYOUTRTL = WS_EX_LAYOUTRTL,
#endif /* WINVER >= 0x0500 */

#if(_WIN32_WINNT >= 0x0501)
        EX_COMPOSITED = WS_EX_COMPOSITED,
#endif /* _WIN32_WINNT >= 0x0501 */
#if(_WIN32_WINNT >= 0x0500)
        EX_NOACTIVATE = WS_EX_NOACTIVATE,
#endif /* _WIN32_WINNT >= 0x0500 */
    };
    using window_style_flags = bitflag<window_style>;

    class window_destroyer
    {
    public:
        using pointer = HWND;

        void operator()(HWND ptr) const;
    };

    class window_class;
    class window
    {
    public:
        PREVENT_COPY_FORCE_MOVE(window);
        window( ) = default;

        struct position_memders
        {
            bool operator==(const position_memders&) const = default;
            int  x, y;
        };
        struct size_memders
        {
            bool operator==(const size_memders&) const = default;
            int  width, height;
        };

        using _Is_created_lazy_t = std::_Front_binder<bool(window::*)( ) const, const window*>;

        template <typename _Ty>
        using _Setter_native = value_setter<_Ty,
                                            _Is_created_lazy_t,
                                            value_setter_native_getter<_Ty>,
                                            value_setter_native_setter<_Ty> >;

        /*using _Setter_string = value_setter<detail::string_info::string,
                                            _Is_created_lazy_t,
                                            default_stable_string::_Lazy_getter,
                                            default_stable_string::_Lazy_setter>;*/
        using _Setter_string = _Setter_native<default_stable_string>;

        auto style( ) -> _Setter_native<window_style_flags>;
        auto style( ) const -> const window_style_flags&;
        auto name( ) -> _Setter_string;
        auto name( ) const -> std::basic_string_view<default_stable_string::value_type>;
        auto position( ) -> _Setter_native<position_memders>;
        auto position( ) const -> const position_memders&;
        auto size( ) -> _Setter_native<size_memders>;
        auto size( ) const -> const size_memders&;

        bool is_created( ) const;
        HWND handle( ) const;

    private:
        _Is_created_lazy_t _Is_created_lazy( ) const;

        //_Setter_string _Init_setter_string(default_stable_string& string_stored) const;

        template <typename _Ty>
        _Setter_native<_Ty> _Init_setter_native(_Ty& val) { return utl::make_value_setter(this->_Is_created_lazy( ), val); }

        class _Controller_const
        {
        public:
            _Controller_const(const window& owner) : owner_(owner) {}

            bool show(int show_command) const;
            bool update( ) const;

        protected:
            const window& owner( ) const;

        private:
            const window& owner_;
        };
        class _Controller: public _Controller_const
        {
        public:
            _Controller(window& owner) : _Controller_const(owner) {}

            void create(const window_class& sample) const;
            void destory( ) const;

        protected:
            window& owner( ) const;
        };

    public:
        _Controller       control( );
        _Controller_const control( ) const;

    private:
        window_style          style_;
        default_stable_string name_;
        position_memders      position_;
        size_memders          size_;

        std::unique_ptr<HWND, window_destroyer> hwnd_;
    };

    class icon_destroyer
    {
    public:
        using pointer = HICON;
        void operator()(HICON ptr) const;
    };

    //using icon = std::unique_ptr<HICON, icon_destroyer>;

    enum class class_types :UINT
    {
        vredraw = CS_VREDRAW,
        hredraw = CS_HREDRAW,
        dblclks = CS_DBLCLKS,
        own_dc = CS_OWNDC,
        class_dc = CS_CLASSDC,
        parent_dc = CS_PARENTDC,
        noclose = CS_NOCLOSE,
        savebits = CS_SAVEBITS,
        byte_align_client = CS_BYTEALIGNCLIENT,
        byte_align_window = CS_BYTEALIGNWINDOW,
        global_class = CS_GLOBALCLASS,

        ime = CS_IME,
#ifdef CS_DROPSHADOW
        drop_shadow = CS_DROPSHADOW,
#endif
    };
    using class_types_flags = bitflag<class_types>;

    class window_class_unregister
    {
    public:
        using pointer = WNDCLASSEX*;
        void operator()(WNDCLASSEX* ptr) const;
    };

    class window_class
    {
    public:
        PREVENT_COPY_FORCE_MOVE(window_class)
        window_class( );

        using _Is_registered_lazy_t = std::_Front_binder<bool(window_class::*)( ) const, const window_class*>;

        template <typename _Ty>
        using _Setter_native = value_setter<_Ty,
                                            _Is_registered_lazy_t,
                                            value_setter_native_getter<_Ty>,
                                            value_setter_native_setter<_Ty> >;

        class _String_to_ptr_binder: public value_setter_native_setter<default_stable_string>
        {
        public:
            _String_to_ptr_binder(default_stable_string& string, default_stable_string::const_pointer& mirror)
                : value_setter_native_setter(string), mirror_(mirror) {}

            ~_String_to_ptr_binder( )
            {
                auto& from = this->get( );
                auto& to   = this->mirror_.get( );
                if (from.empty( ))
                    to = nullptr;
                else
                    to = from.c_str( );
            }

            /*void operator()(default_stable_string&& new_string) const
            {
                auto& ref = string.get( );

                ref        = std::move(new_string);
                ptr.get( ) = ref.c_str( );
            }*/

        private:
            std::reference_wrapper<default_stable_string::const_pointer> mirror_;
        };

        using _Setter_string = value_setter<default_stable_string,
                                            _Is_registered_lazy_t,
                                            value_setter_native_getter<default_stable_string>,
                                            _String_to_ptr_binder>;

        //using _Setter_string = _Setter_native<default_stable_string>;

        auto style( ) -> _Setter_native<class_types_flags>;
        auto style( ) const -> class_types_flags;
        auto wnd_proc( ) -> _Setter_native<WNDPROC>;
        auto wnd_proc( ) const -> WNDPROC;
        auto instance( ) -> _Setter_native<HINSTANCE>;
        auto instance( ) const -> HINSTANCE;
        auto icon( ) -> _Setter_native<HICON>;
        auto icon( ) const -> HICON;
        auto cursor( ) -> _Setter_native<HCURSOR>;
        auto cursor( ) const -> HCURSOR;
        auto background( ) -> _Setter_native<HBRUSH>;
        auto background( ) const -> HBRUSH;
        auto menu_name( ) -> _Setter_string;
        auto menu_name( ) const -> std::basic_string_view<default_stable_string::value_type>;
        auto class_name( ) -> _Setter_string;
        auto class_name( ) const -> std::basic_string_view<default_stable_string::value_type>;
        auto small_icon( ) -> _Setter_native<HICON>;
        auto small_icon( ) const -> HICON;

        bool is_registered( ) const;
        ATOM registered_atom( ) const;

        void register_class( );
        void unregister_class( );

    private:
        _Is_registered_lazy_t _Is_registered_lazy( ) const;
        _Setter_string        _Init_setter_string(default_stable_string& string_stored, default_stable_string::const_pointer& mirror) const;

        template <typename _Ty>
        _Setter_native<_Ty> _Init_setter_native(_Ty& val) { return utl::make_value_setter(this->_Is_registered_lazy( ), val); }

        ATOM                  registered_atom_ = 0;
        default_stable_string menu_name_, class_name_;

        using _Memory_allocator_t = std::allocator_traits<default_stable_string::allocator_type>::rebind_alloc<WNDCLASSEX>;
        using _Memory_allocator_traits = std::allocator_traits<_Memory_allocator_t>;

        class _Allocator: public _Memory_allocator_t
        {
        public:
            using pointer = WNDCLASSEX*;

            void operator()(WNDCLASSEX* ptr)
            {
                this->deallocate(ptr, 1);
            }
        };

        std::unique_ptr<WNDCLASSEX, _Allocator>              memory_;
        std::unique_ptr<WNDCLASSEX, window_class_unregister> registered_;
    };
}
