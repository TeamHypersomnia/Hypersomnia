#if BUILD_WINDOW_FRAMEWORK
#include <Windows.h>
#undef min
#undef max
#include "translate_winapi_enums.h"

using namespace augs::event;
using namespace augs::event::keys;

message translate_enum(const UINT m) {
	switch (m) {
	case SC_CLOSE:						return message::close; 
	case WM_MOVE:						return message::move; 
	case WM_SIZE:						return message::resize; 
	case WM_QUIT:						return message::quit; 
	case SC_MINIMIZE:					return message::minimize; 
	case SC_MAXIMIZE:					return message::maximize; 
	case SC_RESTORE:					return message::restore;  
	case WM_CLIPBOARDUPDATE:			return message::clipboard_change; 
	case WM_SYSKEYDOWN:					return message::syskeydown; 
	case WM_SYSKEYUP:					return message::syskeyup; 
	case WM_KEYDOWN:					return message::keydown; 
	case WM_KEYUP:						return message::keyup; 
	case WM_CHAR:						return message::character; 
	case WM_UNICHAR:					return message::unichar; 
	case WM_MOUSEMOVE:					return message::mousemotion; 
	case WM_MOUSEWHEEL:					return message::wheel; 

	case WM_LBUTTONDBLCLK:				return message::ldoubleclick; 
	case WM_RBUTTONDBLCLK:				return message::rdoubleclick;
	case WM_MBUTTONDBLCLK:				return message::mdoubleclick;

	case WM_LBUTTONDOWN:				return message::keydown;
	case WM_LBUTTONUP:					return message::keyup;
	case WM_RBUTTONDOWN:				return message::keydown;
	case WM_RBUTTONUP:					return message::keyup;
	case WM_MBUTTONDOWN:				return message::keydown; 
	case WM_MBUTTONUP:					return message::keyup; 
	case WM_XBUTTONDOWN:				return message::keydown; 
	case WM_XBUTTONUP:					return message::keyup; 

	case UINT(message::ltripleclick):	return message::ltripleclick;

	default: return augs::event::message::INVALID;
	}
}

key translate_virtual_key(const UINT m) {
	switch (m) {
	case VK_LBUTTON:										return key::LMOUSE;
	case VK_RBUTTON:										return key::RMOUSE;
	case VK_MBUTTON:										return key::MMOUSE;
	case VK_XBUTTON1:										return key::MOUSE4;
	case VK_XBUTTON2:										return key::MOUSE5;
	case VK_CANCEL:											return key::CANCEL;
	case VK_BACK:											return key::BACKSPACE;
	case VK_TAB:											return key::TAB;
	case VK_CLEAR:											return key::CLEAR;
	case VK_RETURN:											return key::ENTER;
	case VK_SHIFT:											return key::SHIFT;
	case VK_CONTROL:										return key::CTRL;
	case VK_MENU:											return key::ALT;
	case VK_PAUSE:											return key::PAUSE;
	case VK_CAPITAL:										return key::CAPSLOCK;
	case VK_ESCAPE:											return key::ESC;
	case VK_SPACE:											return key::SPACE;
	case VK_PRIOR:											return key::PAGEUP;
	case VK_NEXT:											return key::PAGEDOWN;
	case VK_END:											return key::END;
	case VK_HOME:											return key::HOME;
	case VK_LEFT:											return key::LEFT;
	case VK_UP:												return key::UP;
	case VK_RIGHT:											return key::RIGHT;
	case VK_DOWN:											return key::DOWN;
	case VK_SELECT:											return key::SELECT;
	case VK_PRINT:											return key::PRINT;
	case VK_EXECUTE:										return key::EXECUTE;
	case VK_SNAPSHOT:										return key::PRINTSCREEN;
	case VK_INSERT:											return key::INSERT;
	case VK_DELETE:											return key::DEL;
	case VK_HELP:											return key::HELP;
	case VK_LWIN:											return key::LWIN;
	case VK_RWIN:											return key::RWIN;
	case VK_APPS:											return key::APPS;
	case VK_SLEEP:											return key::SLEEP;
	case VK_NUMPAD0:										return key::NUMPAD0;
	case VK_NUMPAD1:										return key::NUMPAD1;
	case VK_NUMPAD2:										return key::NUMPAD2;
	case VK_NUMPAD3:										return key::NUMPAD3;
	case VK_NUMPAD4:										return key::NUMPAD4;
	case VK_NUMPAD5:										return key::NUMPAD5;
	case VK_NUMPAD6:										return key::NUMPAD6;
	case VK_NUMPAD7:										return key::NUMPAD7;
	case VK_NUMPAD8:										return key::NUMPAD8;
	case VK_NUMPAD9:										return key::NUMPAD9;
	case VK_MULTIPLY:										return key::MULTIPLY;
	case VK_ADD:											return key::ADD;
	case VK_SEPARATOR:										return key::SEPARATOR;
	case VK_SUBTRACT:										return key::SUBTRACT;
	case VK_DECIMAL:										return key::DECIMAL;
	case VK_DIVIDE:											return key::DIVIDE;
	case VK_F1:												return key::F1;
	case VK_F2:												return key::F2;
	case VK_F3:												return key::F3;
	case VK_F4:												return key::F4;
	case VK_F5:												return key::F5;
	case VK_F6:												return key::F6;
	case VK_F7:												return key::F7;
	case VK_F8:												return key::F8;
	case VK_F9:												return key::F9;
	case VK_F10:											return key::F10;
	case VK_F11:											return key::F11;
	case VK_F12:											return key::F12;
	case VK_F13:											return key::F13;
	case VK_F14:											return key::F14;
	case VK_F15:											return key::F15;
	case VK_F16:											return key::F16;
	case VK_F17:											return key::F17;
	case VK_F18:											return key::F18;
	case VK_F19:											return key::F19;
	case VK_F20:											return key::F20;
	case VK_F21:											return key::F21;
	case VK_F22:											return key::F22;
	case VK_F23:											return key::F23;
	case VK_F24:											return key::F24;
	case 'A':												return key::A;
	case 'B':												return key::B;
	case 'C':												return key::C;
	case 'D':												return key::D;
	case 'E':												return key::E;
	case 'F':												return key::F;
	case 'G':												return key::G;
	case 'H':												return key::H;
	case 'I':												return key::I;
	case 'J':												return key::J;
	case 'K':												return key::K;
	case 'L':												return key::L;
	case 'M':												return key::M;
	case 'N':												return key::N;
	case 'O':												return key::O;
	case 'P':												return key::P;
	case 'Q':												return key::Q;
	case 'R':												return key::R;
	case 'S':												return key::S;
	case 'T':												return key::T;
	case 'U':												return key::U;
	case 'V':												return key::V;
	case 'W':												return key::W;
	case 'X':												return key::X;
	case 'Y':												return key::Y;
	case 'Z':												return key::Z;
	case '0':												return key::_0;
	case '1':												return key::_1;
	case '2':												return key::_2;
	case '3':												return key::_3;
	case '4':												return key::_4;
	case '5':												return key::_5;
	case '6':												return key::_6;
	case '7':												return key::_7;
	case '8':												return key::_8;
	case '9':												return key::_9;
	case VK_NUMLOCK:										return key::NUMLOCK;
	case VK_SCROLL:											return key::SCROLL;
	case VK_LSHIFT:											return key::LSHIFT;
	case VK_RSHIFT:											return key::RSHIFT;
	case VK_LCONTROL:										return key::LCTRL;
	case VK_RCONTROL:										return key::RCTRL;
	case VK_LMENU:											return key::LALT;
	case VK_RMENU:											return key::RALT;
	case VK_OEM_NEC_EQUAL:									return key::EQUAL;
	case VK_VOLUME_MUTE:									return key::VOLUME_MUTE;
	case VK_VOLUME_UP:										return key::VOLUME_UP;
	case VK_VOLUME_DOWN:									return key::VOLUME_DOWN;
	case VK_MEDIA_NEXT_TRACK:								return key::NEXT_TRACK;
	case VK_MEDIA_PREV_TRACK:								return key::PREV_TRACK;
	case VK_MEDIA_STOP:										return key::STOP_TRACK;
	case VK_MEDIA_PLAY_PAUSE:								return key::PLAY_PAUSE_TRACK;
	case VK_OEM_1:											return key::SEMICOLON;
	case VK_OEM_PLUS:										return key::PLUS;
	case VK_OEM_COMMA:										return key::COMMA;
	case VK_OEM_MINUS:										return key::MINUS;
	case VK_OEM_PERIOD:										return key::PERIOD;
	case VK_OEM_2:											return key::SLASH;
	case VK_OEM_3:											return key::TILDE;
	case VK_OEM_4:											return key::OPEN_SQUARE_BRACKET;
	case VK_OEM_5:											return key::BACKSLASH;
	case VK_OEM_6:											return key::CLOSE_SQUARE_BRACKET;
	case VK_OEM_7:											return key::APOSTROPHE;
	default:												return key::INVALID;
	}
}

key translate_key_with_lparam(const LPARAM lParam, WPARAM m) {
	UINT scancode = (lParam & 0x00ff0000) >> 16;
	int extended = (lParam & 0x01000000) != 0;

	switch (m) {
	case VK_SHIFT:
		m = MapVirtualKey(scancode, MAPVK_VSC_TO_VK_EX);
		break;
	case VK_CONTROL:
		m = extended ? VK_RCONTROL : VK_LCONTROL;
		break;
	case VK_MENU:
		m = extended ? VK_RMENU : VK_LMENU;
		break;
	default:
		break;
	}

	return translate_virtual_key(static_cast<UINT>(m));
}
#endif
