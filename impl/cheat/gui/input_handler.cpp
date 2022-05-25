module;

#define NOMINMAX

#include <cheat/core/object.h>

#include <nstd/runtime_assert.h>

#include <RmlUi/Core.h>

#include <windows.h>
#include <windowsx.h>

#include <functional>
#include <limits>
#include <string_view>

module cheat.gui.input_handler;
import cheat.gui.context;
import nstd.lazy_invoke;
import nstd.text.convert.unicode;

struct input_result_proxy : input_result
{
    using input_result::input_result;

    input_result_proxy(const bool rml_retval)
        : input_result(rml_retval ? processed : interacted)
    {
    }
};

using Rml::Context;

class input_helper
{
    Context* ctx_;

  public:
    input_helper(Context* const ctx);

    input_result_proxy l_button_down(const HWND window);

    input_result_proxy l_button_up();

    input_result_proxy r_button_down();

    input_result_proxy r_button_up();

    input_result_proxy m_button_down();

    input_result_proxy m_button_up();

    input_result_proxy mouse_move(const LPARAM l_param);

    input_result_proxy mouse_wheel(const WPARAM w_param);

    input_result key_down(const WPARAM w_param);
    input_result_proxy key_up(const WPARAM w_param);

    input_result_proxy on_character(const WPARAM w_param, char16_t& first_code_unit);
    input_result resize(const WPARAM w_param, const LPARAM l_param);
};

class input_handler_impl final : public basic_input_handler
{
    Context* ctx_ = nullptr;
    char16_t first_u16_code_unit_ = 0;

    input_helper input() const;

  public:
    void init(Context* const ctx) override;
    input_result operator()(HWND window, UINT message, WPARAM w_param, LPARAM l_param) override;
};

CHEAT_OBJECT_BIND(basic_input_handler, input_handler, input_handler_impl);

input_helper input_handler_impl::input() const
{
    return ctx_;
}

void input_handler_impl::init(Context* const ctx)
{
    runtime_assert(ctx_ && !ctx);
    ctx_ = ctx;
}

input_result input_handler_impl::operator()(HWND window, UINT message, WPARAM w_param, LPARAM l_param)
{
    switch (message)
    {
    case WM_LBUTTONDOWN:
        return input().l_button_down(window).set_return_value(0);
    case WM_LBUTTONUP:
        return input().l_button_up().set_return_value(0);
    case WM_RBUTTONDOWN:
        return input().r_button_down().set_return_value(0);
    case WM_RBUTTONUP:
        return input().r_button_up().set_return_value(0);
    case WM_MBUTTONDOWN:
        return input().m_button_down().set_return_value(0);
    case WM_MBUTTONUP:
        return input().m_button_up().set_return_value(0);
    case WM_MOUSEMOVE:
        return input().mouse_move(l_param).set_return_value(0);
    case WM_MOUSEWHEEL:
        return input().mouse_wheel(w_param).set_return_value(0);
    case WM_KEYDOWN:
        return input().key_down(w_param).set_return_value(0);
    case WM_KEYUP:
        return input().key_up(w_param).set_return_value(0);
    case WM_CHAR:
        return input().on_character(w_param, first_u16_code_unit_).set_return_value(0);
    case WM_SIZE:
        return input().resize(w_param, l_param).set_return_value(0);
    default:
        return input_result::skipped;
    }
}

//--

using namespace Rml;

constexpr auto key_identifier_map = [] {
    std::array<Input::KeyIdentifier, 256> kmap = {};

    // Assign individual values.
    kmap['A'] = Input::KI_A;
    kmap['B'] = Input::KI_B;
    kmap['C'] = Input::KI_C;
    kmap['D'] = Input::KI_D;
    kmap['E'] = Input::KI_E;
    kmap['F'] = Input::KI_F;
    kmap['G'] = Input::KI_G;
    kmap['H'] = Input::KI_H;
    kmap['I'] = Input::KI_I;
    kmap['J'] = Input::KI_J;
    kmap['K'] = Input::KI_K;
    kmap['L'] = Input::KI_L;
    kmap['M'] = Input::KI_M;
    kmap['N'] = Input::KI_N;
    kmap['O'] = Input::KI_O;
    kmap['P'] = Input::KI_P;
    kmap['Q'] = Input::KI_Q;
    kmap['R'] = Input::KI_R;
    kmap['S'] = Input::KI_S;
    kmap['T'] = Input::KI_T;
    kmap['U'] = Input::KI_U;
    kmap['V'] = Input::KI_V;
    kmap['W'] = Input::KI_W;
    kmap['X'] = Input::KI_X;
    kmap['Y'] = Input::KI_Y;
    kmap['Z'] = Input::KI_Z;

    kmap['0'] = Input::KI_0;
    kmap['1'] = Input::KI_1;
    kmap['2'] = Input::KI_2;
    kmap['3'] = Input::KI_3;
    kmap['4'] = Input::KI_4;
    kmap['5'] = Input::KI_5;
    kmap['6'] = Input::KI_6;
    kmap['7'] = Input::KI_7;
    kmap['8'] = Input::KI_8;
    kmap['9'] = Input::KI_9;

    kmap[VK_BACK] = Input::KI_BACK;
    kmap[VK_TAB] = Input::KI_TAB;

    kmap[VK_CLEAR] = Input::KI_CLEAR;
    kmap[VK_RETURN] = Input::KI_RETURN;

    kmap[VK_PAUSE] = Input::KI_PAUSE;
    kmap[VK_CAPITAL] = Input::KI_CAPITAL;

    kmap[VK_KANA] = Input::KI_KANA;
    kmap[VK_HANGUL] = Input::KI_HANGUL;
    kmap[VK_JUNJA] = Input::KI_JUNJA;
    kmap[VK_FINAL] = Input::KI_FINAL;
    kmap[VK_HANJA] = Input::KI_HANJA;
    kmap[VK_KANJI] = Input::KI_KANJI;

    kmap[VK_ESCAPE] = Input::KI_ESCAPE;

    kmap[VK_CONVERT] = Input::KI_CONVERT;
    kmap[VK_NONCONVERT] = Input::KI_NONCONVERT;
    kmap[VK_ACCEPT] = Input::KI_ACCEPT;
    kmap[VK_MODECHANGE] = Input::KI_MODECHANGE;

    kmap[VK_SPACE] = Input::KI_SPACE;
    kmap[VK_PRIOR] = Input::KI_PRIOR;
    kmap[VK_NEXT] = Input::KI_NEXT;
    kmap[VK_END] = Input::KI_END;
    kmap[VK_HOME] = Input::KI_HOME;
    kmap[VK_LEFT] = Input::KI_LEFT;
    kmap[VK_UP] = Input::KI_UP;
    kmap[VK_RIGHT] = Input::KI_RIGHT;
    kmap[VK_DOWN] = Input::KI_DOWN;
    kmap[VK_SELECT] = Input::KI_SELECT;
    kmap[VK_PRINT] = Input::KI_PRINT;
    kmap[VK_EXECUTE] = Input::KI_EXECUTE;
    kmap[VK_SNAPSHOT] = Input::KI_SNAPSHOT;
    kmap[VK_INSERT] = Input::KI_INSERT;
    kmap[VK_DELETE] = Input::KI_DELETE;
    kmap[VK_HELP] = Input::KI_HELP;

    kmap[VK_LWIN] = Input::KI_LWIN;
    kmap[VK_RWIN] = Input::KI_RWIN;
    kmap[VK_APPS] = Input::KI_APPS;

    kmap[VK_SLEEP] = Input::KI_SLEEP;

    kmap[VK_NUMPAD0] = Input::KI_NUMPAD0;
    kmap[VK_NUMPAD1] = Input::KI_NUMPAD1;
    kmap[VK_NUMPAD2] = Input::KI_NUMPAD2;
    kmap[VK_NUMPAD3] = Input::KI_NUMPAD3;
    kmap[VK_NUMPAD4] = Input::KI_NUMPAD4;
    kmap[VK_NUMPAD5] = Input::KI_NUMPAD5;
    kmap[VK_NUMPAD6] = Input::KI_NUMPAD6;
    kmap[VK_NUMPAD7] = Input::KI_NUMPAD7;
    kmap[VK_NUMPAD8] = Input::KI_NUMPAD8;
    kmap[VK_NUMPAD9] = Input::KI_NUMPAD9;
    kmap[VK_MULTIPLY] = Input::KI_MULTIPLY;
    kmap[VK_ADD] = Input::KI_ADD;
    kmap[VK_SEPARATOR] = Input::KI_SEPARATOR;
    kmap[VK_SUBTRACT] = Input::KI_SUBTRACT;
    kmap[VK_DECIMAL] = Input::KI_DECIMAL;
    kmap[VK_DIVIDE] = Input::KI_DIVIDE;
    kmap[VK_F1] = Input::KI_F1;
    kmap[VK_F2] = Input::KI_F2;
    kmap[VK_F3] = Input::KI_F3;
    kmap[VK_F4] = Input::KI_F4;
    kmap[VK_F5] = Input::KI_F5;
    kmap[VK_F6] = Input::KI_F6;
    kmap[VK_F7] = Input::KI_F7;
    kmap[VK_F8] = Input::KI_F8;
    kmap[VK_F9] = Input::KI_F9;
    kmap[VK_F10] = Input::KI_F10;
    kmap[VK_F11] = Input::KI_F11;
    kmap[VK_F12] = Input::KI_F12;
    kmap[VK_F13] = Input::KI_F13;
    kmap[VK_F14] = Input::KI_F14;
    kmap[VK_F15] = Input::KI_F15;
    kmap[VK_F16] = Input::KI_F16;
    kmap[VK_F17] = Input::KI_F17;
    kmap[VK_F18] = Input::KI_F18;
    kmap[VK_F19] = Input::KI_F19;
    kmap[VK_F20] = Input::KI_F20;
    kmap[VK_F21] = Input::KI_F21;
    kmap[VK_F22] = Input::KI_F22;
    kmap[VK_F23] = Input::KI_F23;
    kmap[VK_F24] = Input::KI_F24;

    kmap[VK_NUMLOCK] = Input::KI_NUMLOCK;
    kmap[VK_SCROLL] = Input::KI_SCROLL;

    kmap[VK_OEM_NEC_EQUAL] = Input::KI_OEM_NEC_EQUAL;

    kmap[VK_OEM_FJ_JISHO] = Input::KI_OEM_FJ_JISHO;
    kmap[VK_OEM_FJ_MASSHOU] = Input::KI_OEM_FJ_MASSHOU;
    kmap[VK_OEM_FJ_TOUROKU] = Input::KI_OEM_FJ_TOUROKU;
    kmap[VK_OEM_FJ_LOYA] = Input::KI_OEM_FJ_LOYA;
    kmap[VK_OEM_FJ_ROYA] = Input::KI_OEM_FJ_ROYA;

    kmap[VK_SHIFT] = Input::KI_LSHIFT;
    kmap[VK_CONTROL] = Input::KI_LCONTROL;
    kmap[VK_MENU] = Input::KI_LMENU;

    kmap[VK_BROWSER_BACK] = Input::KI_BROWSER_BACK;
    kmap[VK_BROWSER_FORWARD] = Input::KI_BROWSER_FORWARD;
    kmap[VK_BROWSER_REFRESH] = Input::KI_BROWSER_REFRESH;
    kmap[VK_BROWSER_STOP] = Input::KI_BROWSER_STOP;
    kmap[VK_BROWSER_SEARCH] = Input::KI_BROWSER_SEARCH;
    kmap[VK_BROWSER_FAVORITES] = Input::KI_BROWSER_FAVORITES;
    kmap[VK_BROWSER_HOME] = Input::KI_BROWSER_HOME;

    kmap[VK_VOLUME_MUTE] = Input::KI_VOLUME_MUTE;
    kmap[VK_VOLUME_DOWN] = Input::KI_VOLUME_DOWN;
    kmap[VK_VOLUME_UP] = Input::KI_VOLUME_UP;
    kmap[VK_MEDIA_NEXT_TRACK] = Input::KI_MEDIA_NEXT_TRACK;
    kmap[VK_MEDIA_PREV_TRACK] = Input::KI_MEDIA_PREV_TRACK;
    kmap[VK_MEDIA_STOP] = Input::KI_MEDIA_STOP;
    kmap[VK_MEDIA_PLAY_PAUSE] = Input::KI_MEDIA_PLAY_PAUSE;
    kmap[VK_LAUNCH_MAIL] = Input::KI_LAUNCH_MAIL;
    kmap[VK_LAUNCH_MEDIA_SELECT] = Input::KI_LAUNCH_MEDIA_SELECT;
    kmap[VK_LAUNCH_APP1] = Input::KI_LAUNCH_APP1;
    kmap[VK_LAUNCH_APP2] = Input::KI_LAUNCH_APP2;

    kmap[VK_OEM_1] = Input::KI_OEM_1;
    kmap[VK_OEM_PLUS] = Input::KI_OEM_PLUS;
    kmap[VK_OEM_COMMA] = Input::KI_OEM_COMMA;
    kmap[VK_OEM_MINUS] = Input::KI_OEM_MINUS;
    kmap[VK_OEM_PERIOD] = Input::KI_OEM_PERIOD;
    kmap[VK_OEM_2] = Input::KI_OEM_2;
    kmap[VK_OEM_3] = Input::KI_OEM_3;

    kmap[VK_OEM_4] = Input::KI_OEM_4;
    kmap[VK_OEM_5] = Input::KI_OEM_5;
    kmap[VK_OEM_6] = Input::KI_OEM_6;
    kmap[VK_OEM_7] = Input::KI_OEM_7;
    kmap[VK_OEM_8] = Input::KI_OEM_8;

    kmap[VK_OEM_AX] = Input::KI_OEM_AX;
    kmap[VK_OEM_102] = Input::KI_OEM_102;
    kmap[VK_ICO_HELP] = Input::KI_ICO_HELP;
    kmap[VK_ICO_00] = Input::KI_ICO_00;

    kmap[VK_PROCESSKEY] = Input::KI_PROCESSKEY;

    kmap[VK_ICO_CLEAR] = Input::KI_ICO_CLEAR;

    kmap[VK_ATTN] = Input::KI_ATTN;
    kmap[VK_CRSEL] = Input::KI_CRSEL;
    kmap[VK_EXSEL] = Input::KI_EXSEL;
    kmap[VK_EREOF] = Input::KI_EREOF;
    kmap[VK_PLAY] = Input::KI_PLAY;
    kmap[VK_ZOOM] = Input::KI_ZOOM;
    kmap[VK_PA1] = Input::KI_PA1;
    kmap[VK_OEM_CLEAR] = Input::KI_OEM_CLEAR;

    return kmap;
}();

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

template <typename T, typename... Args>
static auto _Make_string_buff(const T chr, const Args... chars)
{
    std::array<T, sizeof...(Args) + 2> buff;
    auto itr = buff.begin();
    *itr++ = chr;
    ((*itr++ = chars), ...);
    *itr = 0;
    return buff;
}

struct RmlString : String
{
    template <typename T>
    RmlString(T&& obj)
        : String(std::forward<T>(obj))
    {
    }
};

template <typename... Args>
static RmlString _Make_rml_string(const Args... chars)
{
    const auto buff = _Make_string_buff(chars...);
    const std::basic_string_view strv = {buff.data(), buff.size() - 1};
    return nstd::text::convert_to<char>(strv);
}

input_helper::input_helper(Context* const ctx)
    : ctx_(ctx)
{
}

input_result_proxy input_helper::l_button_down(const HWND window)
{
    const nstd::lazy_invoke lazy = std::bind_front(SetCapture, window);
    return ctx_->ProcessMouseButtonDown(0, key_modifier());
}

input_result_proxy input_helper::l_button_up()
{
    ReleaseCapture();
    return ctx_->ProcessMouseButtonUp(0, key_modifier());
}

input_result_proxy input_helper::r_button_down()
{
    return ctx_->ProcessMouseButtonDown(1, key_modifier());
}

input_result_proxy input_helper::r_button_up()
{
    return ctx_->ProcessMouseButtonUp(1, key_modifier());
}

input_result_proxy input_helper::m_button_down()
{
    return ctx_->ProcessMouseButtonDown(2, key_modifier());
}

input_result_proxy input_helper::m_button_up()
{
    return ctx_->ProcessMouseButtonUp(2, key_modifier());
}

input_result_proxy input_helper::mouse_move(const LPARAM l_param)
{
    const auto x = GET_X_LPARAM(l_param);
    const auto y = GET_Y_LPARAM(l_param);
    return ctx_->ProcessMouseMove(x, y, key_modifier());
}

input_result_proxy input_helper::mouse_wheel(const WPARAM w_param)
{
    const auto delta = GET_WHEEL_DELTA_WPARAM(w_param);
    const auto state = GET_KEYSTATE_WPARAM(w_param);
    return ctx_->ProcessMouseWheel(delta, state);
}

input_result input_helper::key_down(const WPARAM w_param)
{
    const auto key_identifier = key_identifier_map[w_param];
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

input_result_proxy input_helper::key_up(const WPARAM w_param)
{
    return ctx_->ProcessKeyUp(key_identifier_map[w_param], key_modifier());
}

input_result_proxy input_helper::on_character(const WPARAM w_param, char16_t& first_code_unit)
{
    const auto wch = static_cast<char16_t>(w_param);

    if (IS_LOW_SURROGATE(w_param))
    {
        first_code_unit = wch;
        return input_result::processed;
    }
    if (IS_HIGH_SURROGATE(w_param))
    {
        runtime_assert(IS_LOW_SURROGATE(first_code_unit));
        return ctx_->ProcessTextInput(_Make_rml_string(first_code_unit, wch));
    }

    constexpr char16_t first_valid_chr = 32;
    constexpr char16_t last_valid_chr = 126;
    constexpr char16_t last_byte_char = std::numeric_limits<char>::max();

    switch (wch)
    {
    case '\n':
    case '\r':
        return ctx_->ProcessTextInput(_Make_rml_string('\n'));
    case last_byte_char: // delete character
        return input_result::processed;
    default: {
        if (wch < first_valid_chr)
            return input_result::processed;
        String str;
        if (wch <= last_valid_chr)
            str += static_cast<char>(wch);
        else
            str = _Make_rml_string(wch);
        return ctx_->ProcessTextInput(str);
    }
    }
}

input_result input_helper::resize(const WPARAM w_param, const LPARAM l_param)
{
    if (w_param == SIZE_MINIMIZED)
        return input_result::skipped;
    /*const int width = LOWORD(l_param);
    const int height = HIWORD(l_param);*/
    const auto width = GET_X_LPARAM(l_param);
    const auto height = GET_Y_LPARAM(l_param);
    ctx_->SetDimensions({width, height});
    return input_result::processed;
}

//--

input_result::input_result(const result val)
    : result_(val)
{
}

input_result::operator bool() const
{
    return result_ != result::skipped;
}

bool input_result::touched() const
{
    return result_ == result::interacted;
}

input_result& input_result::set_return_value(const uint8_t ret_val)
{
    runtime_assert(ret_val_ == unset_, "Already set!");
    ret_val_ = ret_val;
    return *this;
}

size_t input_result::return_value() const
{
    runtime_assert(touched(), "Unable to return untouched result!");
    runtime_assert(ret_val_ == 1 || ret_val_ == 0, "Incorrect value!");
    return static_cast<size_t>(ret_val_);
}
