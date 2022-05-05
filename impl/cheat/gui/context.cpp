﻿module;

#include <nstd/core.h>

#include <RmlUi/Core.h>
#ifdef _DEBUG
#include <RmlUi/Debugger.h>
#endif

#include <d3d9.h>
#include <windows.h>

#include <memory>

module cheat.gui.context;
import cheat.gui.render_interface;
import cheat.gui.system_interface;
import nstd.one_instance;
import nstd.text.convert;

using namespace cheat::gui;
using namespace Rml;

input_result::input_result(const result val)
	:result_(val)
{
}

input_result::operator bool( ) const noexcept
{
	return result_ != result::skipped;
}

bool input_result::touched( ) const noexcept
{
	return result_ == result::interacted;
}

#define RML_ASSERT_DIR NSTD_CONCAT(RMLUI_DIR, \Samples\assets\)
#define RML_SAMPLE(_S_) NSTD_CONCAT(NSTD_STRINGIZE_RAW(RML_ASSERT_DIR),_S_)

context::context( )
{
	SetRenderInterface(&nstd::instance_of<render_interface>);
	SetSystemInterface(&nstd::instance_of<system_interface>);

	Initialise( );

	//------

	D3DDEVICE_CREATION_PARAMETERS params;
	nstd::instance_of<IDirect3DDevice9*>->GetCreationParameters(std::addressof(params));
	info_.window = params.hFocusWindow;
	RECT rect;
	/*GetWindowRect*/GetClientRect(params.hFocusWindow, std::addressof(rect));
	const int width = rect.right - rect.left;
	const int height = rect.bottom - rect.top;
	ctx_ = CreateContext("main", {width, height});

	//------

#ifdef _DEBUG
	Debugger::Initialise(ctx_);
	Debugger::SetVisible(true);
#endif

	// Tell RmlUi to load the given fonts.
	LoadFontFace(RML_SAMPLE("LatoLatin-Regular.ttf"));
	// Fonts can be registered as fallback fonts, as in this case to display emojis.
	LoadFontFace(RML_SAMPLE("NotoEmoji-Regular.ttf"), true);

	LoadFontFace("C:/Windows/fonts/arial.ttf", true);

	ctx_->LoadDocument(RML_SAMPLE("demo.rml"))->Show( );
	ctx_->LoadDocument(RML_SAMPLE("window.rml"))->Show( );

#if 0
	// Replace and style some text in the loaded document.
	Rml::Element* element = document->GetElementById("world");
	element->SetInnerRML(reinterpret_cast<const char*>(u8"🌍"));
	element->SetProperty("font-size", "1.5em");

	// Set up data bindings to synchronize application data.
	if(Rml::DataModelConstructor constructor = context->CreateDataModel("animals"))
	{
		constructor.Bind("show_text", &my_data.show_text);
		constructor.Bind("animal", &my_data.animal);
	}
#endif
}

context::~context( )
{
	//RemoveContext(ctx_->GetName( ));
	Shutdown( );
}

Context* context::operator->( ) const noexcept
{
	return ctx_;
}

const context_info& context::get_info( ) const noexcept
{
	return info_;
}

void context::render( )
{
	nstd::instance_of<render_interface>->RenderContext(ctx_);
}

constexpr auto key_identifier_map = []
{
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

static int key_modifier( ) noexcept
{
	int key_modifier_state = 0;

	// Query the state of all modifier keys
	if(GetKeyState(VK_CAPITAL) & 1)
	{
		key_modifier_state |= Input::KM_CAPSLOCK;
	}

	if(HIWORD(GetKeyState(VK_SHIFT)) & 1)
	{
		key_modifier_state |= Input::KM_SHIFT;
	}

	if(GetKeyState(VK_NUMLOCK) & 1)
	{
		key_modifier_state |= Input::KM_NUMLOCK;
	}

	if(HIWORD(GetKeyState(VK_CONTROL)) & 1)
	{
		key_modifier_state |= Input::KM_CTRL;
	}

	if(HIWORD(GetKeyState(VK_MENU)) & 1)
	{
		key_modifier_state |= Input::KM_ALT;
	}

	return key_modifier_state;
}

input_result context::input(HWND window, UINT message, WPARAM w_param, LPARAM l_param)
{
	RMLUI_ASSERT(info_.window == window);

	using result = input_result;

	switch(message)
	{
		case WM_LBUTTONDOWN:
		{
			const bool ret = ctx_->ProcessMouseButtonDown(0, key_modifier( ));
			SetCapture(window);
			return ret ? result::processed : result::interacted;
		}
		case WM_LBUTTONUP:
		{
			ReleaseCapture( );
			return ctx_->ProcessMouseButtonUp(0, key_modifier( )) ? result::processed : result::interacted;
		}
		case WM_RBUTTONDOWN:
			return ctx_->ProcessMouseButtonDown(1, key_modifier( )) ? result::processed : result::interacted;
		case WM_RBUTTONUP:
			return ctx_->ProcessMouseButtonUp(1, key_modifier( )) ? result::processed : result::interacted;
		case WM_MBUTTONDOWN:
			return ctx_->ProcessMouseButtonDown(2, key_modifier( )) ? result::processed : result::interacted;
		case WM_MBUTTONUP:
			return ctx_->ProcessMouseButtonUp(2, key_modifier( )) ? result::processed : result::interacted;
		case WM_MOUSEMOVE:
			return ctx_->ProcessMouseMove(static_cast<int>((short)LOWORD(l_param)), static_cast<int>((short)HIWORD(l_param)), key_modifier( )) ? result::processed : result::interacted;
		case WM_MOUSEWHEEL:
			return ctx_->ProcessMouseWheel(static_cast<float>((short)HIWORD(w_param)) / static_cast<float>(-WHEEL_DELTA), key_modifier( )) ? result::processed : result::interacted;
		case WM_KEYDOWN:
		{
			Input::KeyIdentifier key_identifier = key_identifier_map[w_param];
			const int key_modifier_state = key_modifier( );
#if 0
			// Toggle debugger and set 'dp'-ratio ctrl +/-/0 keys. These global shortcuts take priority.
			if(key_identifier == Input::KI_F8)
			{
				Debugger::SetVisible(!Debugger::IsVisible( ));
			}
			else if(key_identifier == Input::KI_0 && key_modifier_state & Input::KM_CTRL)
			{
				ctx_->SetDensityIndependentPixelRatio(Shell::GetDensityIndependentPixelRatio( ));
			}
			else if(key_identifier == Input::KI_1 && key_modifier_state & Input::KM_CTRL)
			{
				ctx_->SetDensityIndependentPixelRatio(1.f);
			}
			else if(key_identifier == Input::KI_OEM_MINUS && key_modifier_state & Input::KM_CTRL)
			{
				const float new_dp_ratio = Math::Max(ctx_->GetDensityIndependentPixelRatio( ) / 1.2f, 0.5f);
				ctx_->SetDensityIndependentPixelRatio(new_dp_ratio);
			}
			else if(key_identifier == Input::KI_OEM_PLUS && key_modifier_state & Input::KM_CTRL)
			{
				const float new_dp_ratio = Math::Min(ctx_->GetDensityIndependentPixelRatio( ) * 1.2f, 2.5f);
				ctx_->SetDensityIndependentPixelRatio(new_dp_ratio);
			}
			else
			{
			}
#endif

			// No global shortcuts detected, submit the key to the ctx_.
			if(!ctx_->ProcessKeyDown(key_identifier, key_modifier_state))
				return result::interacted;

			// The key was not consumed, check for shortcuts that are of lower priority.
			if(key_identifier == Input::KI_R && key_modifier_state & Input::KM_CTRL)
			{
				for(int i = 0; i < ctx_->GetNumDocuments( ); i++)
				{
					ElementDocument* document = ctx_->GetDocument(i);
					if(document->GetSourceURL( ).ends_with(".rml"))
						document->ReloadStyleSheet( );

				}
			}
			return result::processed;
		}
		case WM_KEYUP:
			return ctx_->ProcessKeyUp(key_identifier_map[w_param], key_modifier( )) ? result::processed : result::interacted;
		case WM_CHAR:
		{
			const wchar_t c = (wchar_t)w_param;
			Character character = (Character)c;

			// Windows sends two-wide characters as two messages.
			if(c >= 0xD800 && c < 0xDC00)
			{
				// First 16-bit code unit of a two-wide character.
				first_u16_code_unit_ = c;
			}
			else
			{
				// Second 16-bit code unit of a two-wide character.
				if(c >= 0xDC00 && c < 0xE000 && first_u16_code_unit_ != 0)
				{

					wchar_t str[3];
					str[0] = first_u16_code_unit_;
					str[1] = c;
					str[2] = 0;

					using nstd::text::convert_to;
					String utf8 = /*ConvertToUTF8*/convert_to<char>(std::wstring_view(str, 2));
					character = StringUtilities::ToCharacter(utf8.data( ));
					auto test = convert_to<char32_t>(utf8);
					auto test0 = test[0];
					DebugBreak( );
				}
				else if(c == '\r')
				{
					// Windows sends new-lines as carriage returns, convert to endlines.
					character = (Character)'\n';
				}

				first_u16_code_unit_ = 0;

				// Only send through printable characters.
				if(((char32_t)character >= 32 || character == (Character)'\n') && character != (Character)127)
				{
					return ctx_->ProcessTextInput(character) ? result::processed : result::interacted;
				}
			}
			return result::processed;
		}

		//-------------

		case WM_SIZE:
		{
			if(w_param == SIZE_MINIMIZED)
				return result::skipped;
			const int width = LOWORD(l_param);;
			const int height = HIWORD(l_param);
			ctx_->SetDimensions({width,height});
			//nstd::instance_of<render_interface>->ReleaseTextures( );
			return result::processed;
		}
		default:
			return result::skipped;
	}
}