
#include <fd/assert.h>
#include <fd/object.h>

#include <RmlUi/Core.h>

#include <windows.h>
#include <windowsx.h>

#include <functional>
#include <limits>
#include <span>

import fd.gui.basic_input_handler;
import fd.lazy_invoke;
import fd.convert_to;
import fd.string;

using namespace Rml;

//---------------

using fd::gui::input_result;

struct input_result_proxy : input_result
{
    using input_result::input_result;

    input_result_proxy(const bool rml_retval)
        : input_result(rml_retval ? processed : interacted)
    {
    }
};

//---------------

struct RmlString : String
{
    RmlString() = default;

    template <typename T, typename... Args>
    RmlString(const T chr, const Args... chars)
    {
        std::array<T, sizeof...(Args) + 2> buff;
        auto itr = buff.begin();
        *itr++   = chr;
        ((*itr++ = chars), ...);
        *itr = 0;

        const fd::basic_string_view strv = { buff.data(), buff.size() - 1 };
        this->assign(fd::convert_to<char>(strv));
    }
};

class input_helper
{
#ifdef _DEBUG
    std::span<const Input::KeyIdentifier, 256>
#else
    const Input::KeyIdentifier*
#endif
        key_identifier_map;
    Context* ctx_;

    static int key_modifier()
    {
        int key_modifier_state = 0;

        // Query the state of all modifier keys
        if (GetKeyState(VK_CAPITAL) & 1)
        {
            key_modifier_state |= Input::KM_CAPSLOCK;
        }

        if (HIWORD(GetKeyState(VK_SHIFT)) & 1)
        {
            key_modifier_state |= Input::KM_SHIFT;
        }

        if (GetKeyState(VK_NUMLOCK) & 1)
        {
            key_modifier_state |= Input::KM_NUMLOCK;
        }

        if (HIWORD(GetKeyState(VK_CONTROL)) & 1)
        {
            key_modifier_state |= Input::KM_CTRL;
        }

        if (HIWORD(GetKeyState(VK_MENU)) & 1)
        {
            key_modifier_state |= Input::KM_ALT;
        }

        return key_modifier_state;
    }

  public:
    input_helper(const Input::KeyIdentifier* keys_map, Context* const ctx)
        : key_identifier_map(keys_map, 256)
        , ctx_(ctx)
    {
    }

    input_result_proxy l_button_down(const HWND window)
    {
        const fd::lazy_invoke lazy = std::bind_front(SetCapture, window);
        return ctx_->ProcessMouseButtonDown(0, key_modifier());
    }

    input_result_proxy l_button_up()
    {
        ReleaseCapture();
        return ctx_->ProcessMouseButtonUp(0, key_modifier());
    }

    input_result_proxy r_button_down()
    {
        return ctx_->ProcessMouseButtonDown(1, key_modifier());
    }

    input_result_proxy r_button_up()
    {
        return ctx_->ProcessMouseButtonUp(1, key_modifier());
    }

    input_result_proxy m_button_down()
    {
        return ctx_->ProcessMouseButtonDown(2, key_modifier());
    }

    input_result_proxy m_button_up()
    {
        return ctx_->ProcessMouseButtonUp(2, key_modifier());
    }

    input_result_proxy mouse_move(const LPARAM l_param)
    {
        const auto x = GET_X_LPARAM(l_param);
        const auto y = GET_Y_LPARAM(l_param);
        return ctx_->ProcessMouseMove(x, y, key_modifier());
    }

    input_result_proxy mouse_wheel(const WPARAM w_param)
    {
        const auto delta = GET_WHEEL_DELTA_WPARAM(w_param);
        const auto state = GET_KEYSTATE_WPARAM(w_param);
        return ctx_->ProcessMouseWheel(delta, state);
    }

    input_result key_down(const WPARAM w_param)
    {
        const auto key_identifier    = key_identifier_map[w_param];
        const int key_modifier_state = key_modifier();
#if 0
        // Toggle debugger and set 'dp'-ratio ctrl +/-/0 keys. These global shortcuts take priority.
        if (key_identifier == Input::KI_F8)
        {
            Debugger::SetVisible(!Debugger::IsVisible());
        }
        else if (key_identifier == Input::KI_0 && key_modifier_state & Input::KM_CTRL)
        {
            ctx_->SetDensityIndependentPixelRatio(Shell::GetDensityIndependentPixelRatio());
        }
        else if (key_identifier == Input::KI_1 && key_modifier_state & Input::KM_CTRL)
        {
            ctx_->SetDensityIndependentPixelRatio(1.f);
        }
        else if (key_identifier == Input::KI_OEM_MINUS && key_modifier_state & Input::KM_CTRL)
        {
            const float new_dp_ratio = Math::Max(ctx_->GetDensityIndependentPixelRatio() / 1.2f, 0.5f);
            ctx_->SetDensityIndependentPixelRatio(new_dp_ratio);
        }
        else if (key_identifier == Input::KI_OEM_PLUS && key_modifier_state & Input::KM_CTRL)
        {
            const float new_dp_ratio = Math::Min(ctx_->GetDensityIndependentPixelRatio() * 1.2f, 2.5f);
            ctx_->SetDensityIndependentPixelRatio(new_dp_ratio);
        }
        else
        {
        }
#endif

        // No global shortcuts detected, submit the key to the ctx_.
        if (!ctx_->ProcessKeyDown(key_identifier, key_modifier_state))
            return input_result::interacted;

        // The key was not consumed, check for shortcuts that are of lower priority.
        if (key_identifier == Input::KI_R && key_modifier_state & Input::KM_CTRL)
        {
            for (int i = 0; i < ctx_->GetNumDocuments(); i++)
            {
                ElementDocument* document = ctx_->GetDocument(i);
                if (document->GetSourceURL().ends_with(".rml"))
                    document->ReloadStyleSheet();
            }
        }
        return input_result::processed;
    }

    input_result_proxy key_up(const WPARAM w_param)
    {
        return ctx_->ProcessKeyUp(key_identifier_map[w_param], key_modifier());
    }

    input_result_proxy on_character(const WPARAM w_param, char16_t& first_code_unit)
    {
        const auto wch = static_cast<char16_t>(w_param);

        if (IS_LOW_SURROGATE(w_param))
        {
            first_code_unit = wch;
            return input_result::processed;
        }
        if (IS_HIGH_SURROGATE(w_param))
        {
            FD_ASSERT(IS_LOW_SURROGATE(first_code_unit));
            return ctx_->ProcessTextInput(RmlString(first_code_unit, wch));
        }

        constexpr char16_t first_valid_chr = 32;
        constexpr char16_t last_valid_chr  = 126;
        constexpr char16_t last_byte_char  = std::numeric_limits<char>::max();

        switch (wch)
        {
        case '\n':
        case '\r':
            return ctx_->ProcessTextInput(RmlString('\n'));
        case last_byte_char: // delete character
            return input_result::processed;
        default: {
            if (wch < first_valid_chr)
                return input_result::processed;
            String str;
            if (wch <= last_valid_chr)
                str += static_cast<char>(wch);
            else
                str = RmlString(wch);
            return ctx_->ProcessTextInput(str);
        }
        }
    }

    input_result resize(const WPARAM w_param, const LPARAM l_param)
    {
        if (w_param == SIZE_MINIMIZED)
            return input_result::skipped;
        /*const int width = LOWORD(l_param);
        const int height = HIWORD(l_param);*/
        const auto width  = GET_X_LPARAM(l_param);
        const auto height = GET_Y_LPARAM(l_param);
        ctx_->SetDimensions({ width, height });
        return input_result::processed;
    }
};

//-----------

class input_handler_impl final : public fd::gui::basic_input_handler
{
    std::array<Input::KeyIdentifier, 256> keys_map_;
    Context* ctx_                 = nullptr;
    char16_t first_u16_code_unit_ = 0;

    void _Init_keys()
    {
        // Assign individual values.
        keys_map_['A'] = Input::KI_A;
        keys_map_['B'] = Input::KI_B;
        keys_map_['C'] = Input::KI_C;
        keys_map_['D'] = Input::KI_D;
        keys_map_['E'] = Input::KI_E;
        keys_map_['F'] = Input::KI_F;
        keys_map_['G'] = Input::KI_G;
        keys_map_['H'] = Input::KI_H;
        keys_map_['I'] = Input::KI_I;
        keys_map_['J'] = Input::KI_J;
        keys_map_['K'] = Input::KI_K;
        keys_map_['L'] = Input::KI_L;
        keys_map_['M'] = Input::KI_M;
        keys_map_['N'] = Input::KI_N;
        keys_map_['O'] = Input::KI_O;
        keys_map_['P'] = Input::KI_P;
        keys_map_['Q'] = Input::KI_Q;
        keys_map_['R'] = Input::KI_R;
        keys_map_['S'] = Input::KI_S;
        keys_map_['T'] = Input::KI_T;
        keys_map_['U'] = Input::KI_U;
        keys_map_['V'] = Input::KI_V;
        keys_map_['W'] = Input::KI_W;
        keys_map_['X'] = Input::KI_X;
        keys_map_['Y'] = Input::KI_Y;
        keys_map_['Z'] = Input::KI_Z;

        keys_map_['0'] = Input::KI_0;
        keys_map_['1'] = Input::KI_1;
        keys_map_['2'] = Input::KI_2;
        keys_map_['3'] = Input::KI_3;
        keys_map_['4'] = Input::KI_4;
        keys_map_['5'] = Input::KI_5;
        keys_map_['6'] = Input::KI_6;
        keys_map_['7'] = Input::KI_7;
        keys_map_['8'] = Input::KI_8;
        keys_map_['9'] = Input::KI_9;

        keys_map_[VK_BACK] = Input::KI_BACK;
        keys_map_[VK_TAB]  = Input::KI_TAB;

        keys_map_[VK_CLEAR]  = Input::KI_CLEAR;
        keys_map_[VK_RETURN] = Input::KI_RETURN;

        keys_map_[VK_PAUSE]   = Input::KI_PAUSE;
        keys_map_[VK_CAPITAL] = Input::KI_CAPITAL;

        keys_map_[VK_KANA]   = Input::KI_KANA;
        keys_map_[VK_HANGUL] = Input::KI_HANGUL;
        keys_map_[VK_JUNJA]  = Input::KI_JUNJA;
        keys_map_[VK_FINAL]  = Input::KI_FINAL;
        keys_map_[VK_HANJA]  = Input::KI_HANJA;
        keys_map_[VK_KANJI]  = Input::KI_KANJI;

        keys_map_[VK_ESCAPE] = Input::KI_ESCAPE;

        keys_map_[VK_CONVERT]    = Input::KI_CONVERT;
        keys_map_[VK_NONCONVERT] = Input::KI_NONCONVERT;
        keys_map_[VK_ACCEPT]     = Input::KI_ACCEPT;
        keys_map_[VK_MODECHANGE] = Input::KI_MODECHANGE;

        keys_map_[VK_SPACE]    = Input::KI_SPACE;
        keys_map_[VK_PRIOR]    = Input::KI_PRIOR;
        keys_map_[VK_NEXT]     = Input::KI_NEXT;
        keys_map_[VK_END]      = Input::KI_END;
        keys_map_[VK_HOME]     = Input::KI_HOME;
        keys_map_[VK_LEFT]     = Input::KI_LEFT;
        keys_map_[VK_UP]       = Input::KI_UP;
        keys_map_[VK_RIGHT]    = Input::KI_RIGHT;
        keys_map_[VK_DOWN]     = Input::KI_DOWN;
        keys_map_[VK_SELECT]   = Input::KI_SELECT;
        keys_map_[VK_PRINT]    = Input::KI_PRINT;
        keys_map_[VK_EXECUTE]  = Input::KI_EXECUTE;
        keys_map_[VK_SNAPSHOT] = Input::KI_SNAPSHOT;
        keys_map_[VK_INSERT]   = Input::KI_INSERT;
        keys_map_[VK_DELETE]   = Input::KI_DELETE;
        keys_map_[VK_HELP]     = Input::KI_HELP;

        keys_map_[VK_LWIN] = Input::KI_LWIN;
        keys_map_[VK_RWIN] = Input::KI_RWIN;
        keys_map_[VK_APPS] = Input::KI_APPS;

        keys_map_[VK_SLEEP] = Input::KI_SLEEP;

        keys_map_[VK_NUMPAD0]   = Input::KI_NUMPAD0;
        keys_map_[VK_NUMPAD1]   = Input::KI_NUMPAD1;
        keys_map_[VK_NUMPAD2]   = Input::KI_NUMPAD2;
        keys_map_[VK_NUMPAD3]   = Input::KI_NUMPAD3;
        keys_map_[VK_NUMPAD4]   = Input::KI_NUMPAD4;
        keys_map_[VK_NUMPAD5]   = Input::KI_NUMPAD5;
        keys_map_[VK_NUMPAD6]   = Input::KI_NUMPAD6;
        keys_map_[VK_NUMPAD7]   = Input::KI_NUMPAD7;
        keys_map_[VK_NUMPAD8]   = Input::KI_NUMPAD8;
        keys_map_[VK_NUMPAD9]   = Input::KI_NUMPAD9;
        keys_map_[VK_MULTIPLY]  = Input::KI_MULTIPLY;
        keys_map_[VK_ADD]       = Input::KI_ADD;
        keys_map_[VK_SEPARATOR] = Input::KI_SEPARATOR;
        keys_map_[VK_SUBTRACT]  = Input::KI_SUBTRACT;
        keys_map_[VK_DECIMAL]   = Input::KI_DECIMAL;
        keys_map_[VK_DIVIDE]    = Input::KI_DIVIDE;
        keys_map_[VK_F1]        = Input::KI_F1;
        keys_map_[VK_F2]        = Input::KI_F2;
        keys_map_[VK_F3]        = Input::KI_F3;
        keys_map_[VK_F4]        = Input::KI_F4;
        keys_map_[VK_F5]        = Input::KI_F5;
        keys_map_[VK_F6]        = Input::KI_F6;
        keys_map_[VK_F7]        = Input::KI_F7;
        keys_map_[VK_F8]        = Input::KI_F8;
        keys_map_[VK_F9]        = Input::KI_F9;
        keys_map_[VK_F10]       = Input::KI_F10;
        keys_map_[VK_F11]       = Input::KI_F11;
        keys_map_[VK_F12]       = Input::KI_F12;
        keys_map_[VK_F13]       = Input::KI_F13;
        keys_map_[VK_F14]       = Input::KI_F14;
        keys_map_[VK_F15]       = Input::KI_F15;
        keys_map_[VK_F16]       = Input::KI_F16;
        keys_map_[VK_F17]       = Input::KI_F17;
        keys_map_[VK_F18]       = Input::KI_F18;
        keys_map_[VK_F19]       = Input::KI_F19;
        keys_map_[VK_F20]       = Input::KI_F20;
        keys_map_[VK_F21]       = Input::KI_F21;
        keys_map_[VK_F22]       = Input::KI_F22;
        keys_map_[VK_F23]       = Input::KI_F23;
        keys_map_[VK_F24]       = Input::KI_F24;

        keys_map_[VK_NUMLOCK] = Input::KI_NUMLOCK;
        keys_map_[VK_SCROLL]  = Input::KI_SCROLL;

        keys_map_[VK_OEM_NEC_EQUAL] = Input::KI_OEM_NEC_EQUAL;

        keys_map_[VK_OEM_FJ_JISHO]   = Input::KI_OEM_FJ_JISHO;
        keys_map_[VK_OEM_FJ_MASSHOU] = Input::KI_OEM_FJ_MASSHOU;
        keys_map_[VK_OEM_FJ_TOUROKU] = Input::KI_OEM_FJ_TOUROKU;
        keys_map_[VK_OEM_FJ_LOYA]    = Input::KI_OEM_FJ_LOYA;
        keys_map_[VK_OEM_FJ_ROYA]    = Input::KI_OEM_FJ_ROYA;

        keys_map_[VK_SHIFT]   = Input::KI_LSHIFT;
        keys_map_[VK_CONTROL] = Input::KI_LCONTROL;
        keys_map_[VK_MENU]    = Input::KI_LMENU;

        keys_map_[VK_BROWSER_BACK]      = Input::KI_BROWSER_BACK;
        keys_map_[VK_BROWSER_FORWARD]   = Input::KI_BROWSER_FORWARD;
        keys_map_[VK_BROWSER_REFRESH]   = Input::KI_BROWSER_REFRESH;
        keys_map_[VK_BROWSER_STOP]      = Input::KI_BROWSER_STOP;
        keys_map_[VK_BROWSER_SEARCH]    = Input::KI_BROWSER_SEARCH;
        keys_map_[VK_BROWSER_FAVORITES] = Input::KI_BROWSER_FAVORITES;
        keys_map_[VK_BROWSER_HOME]      = Input::KI_BROWSER_HOME;

        keys_map_[VK_VOLUME_MUTE]         = Input::KI_VOLUME_MUTE;
        keys_map_[VK_VOLUME_DOWN]         = Input::KI_VOLUME_DOWN;
        keys_map_[VK_VOLUME_UP]           = Input::KI_VOLUME_UP;
        keys_map_[VK_MEDIA_NEXT_TRACK]    = Input::KI_MEDIA_NEXT_TRACK;
        keys_map_[VK_MEDIA_PREV_TRACK]    = Input::KI_MEDIA_PREV_TRACK;
        keys_map_[VK_MEDIA_STOP]          = Input::KI_MEDIA_STOP;
        keys_map_[VK_MEDIA_PLAY_PAUSE]    = Input::KI_MEDIA_PLAY_PAUSE;
        keys_map_[VK_LAUNCH_MAIL]         = Input::KI_LAUNCH_MAIL;
        keys_map_[VK_LAUNCH_MEDIA_SELECT] = Input::KI_LAUNCH_MEDIA_SELECT;
        keys_map_[VK_LAUNCH_APP1]         = Input::KI_LAUNCH_APP1;
        keys_map_[VK_LAUNCH_APP2]         = Input::KI_LAUNCH_APP2;

        keys_map_[VK_OEM_1]      = Input::KI_OEM_1;
        keys_map_[VK_OEM_PLUS]   = Input::KI_OEM_PLUS;
        keys_map_[VK_OEM_COMMA]  = Input::KI_OEM_COMMA;
        keys_map_[VK_OEM_MINUS]  = Input::KI_OEM_MINUS;
        keys_map_[VK_OEM_PERIOD] = Input::KI_OEM_PERIOD;
        keys_map_[VK_OEM_2]      = Input::KI_OEM_2;
        keys_map_[VK_OEM_3]      = Input::KI_OEM_3;

        keys_map_[VK_OEM_4] = Input::KI_OEM_4;
        keys_map_[VK_OEM_5] = Input::KI_OEM_5;
        keys_map_[VK_OEM_6] = Input::KI_OEM_6;
        keys_map_[VK_OEM_7] = Input::KI_OEM_7;
        keys_map_[VK_OEM_8] = Input::KI_OEM_8;

        keys_map_[VK_OEM_AX]   = Input::KI_OEM_AX;
        keys_map_[VK_OEM_102]  = Input::KI_OEM_102;
        keys_map_[VK_ICO_HELP] = Input::KI_ICO_HELP;
        keys_map_[VK_ICO_00]   = Input::KI_ICO_00;

        keys_map_[VK_PROCESSKEY] = Input::KI_PROCESSKEY;

        keys_map_[VK_ICO_CLEAR] = Input::KI_ICO_CLEAR;

        keys_map_[VK_ATTN]      = Input::KI_ATTN;
        keys_map_[VK_CRSEL]     = Input::KI_CRSEL;
        keys_map_[VK_EXSEL]     = Input::KI_EXSEL;
        keys_map_[VK_EREOF]     = Input::KI_EREOF;
        keys_map_[VK_PLAY]      = Input::KI_PLAY;
        keys_map_[VK_ZOOM]      = Input::KI_ZOOM;
        keys_map_[VK_PA1]       = Input::KI_PA1;
        keys_map_[VK_OEM_CLEAR] = Input::KI_OEM_CLEAR;
    }

    input_helper input() const
    {
        return { keys_map_.data(), ctx_ };
    }

  public:
    input_handler_impl()
        : ctx_(&FD_OBJECT_GET(Rml::Context*))
    {
        _Init_keys();
    }

    input_result operator()(const HWND window, const UINT message, const WPARAM w_param, const LPARAM l_param) override
    {
        switch (message)
        {
        case WM_LBUTTONDOWN:
            return input().l_button_down(window) /*.set_return_value(0)*/;
        case WM_LBUTTONUP:
            return input().l_button_up() /*.set_return_value(0)*/;
        case WM_RBUTTONDOWN:
            return input().r_button_down() /*.set_return_value(0)*/;
        case WM_RBUTTONUP:
            return input().r_button_up() /*.set_return_value(0)*/;
        case WM_MBUTTONDOWN:
            return input().m_button_down() /*.set_return_value(0)*/;
        case WM_MBUTTONUP:
            return input().m_button_up() /*.set_return_value(0)*/;
        case WM_MOUSEMOVE:
            return input().mouse_move(l_param) /*.set_return_value(0)*/;
        case WM_MOUSEWHEEL:
            return input().mouse_wheel(w_param) /*.set_return_value(0)*/;
        case WM_KEYDOWN:
            return input().key_down(w_param) /*.set_return_value(0)*/;
        case WM_KEYUP:
            return input().key_up(w_param) /*.set_return_value(0)*/;
        case WM_CHAR:
            return input().on_character(w_param, first_u16_code_unit_) /*.set_return_value(0)*/;
        case WM_SIZE:
            return input().resize(w_param, l_param) /*.set_return_value(0)*/;
        default:
            return input_result::skipped;
        }
    }
};

FD_OBJECT_BIND_TYPE(fd::gui::input_handler, input_handler_impl);
