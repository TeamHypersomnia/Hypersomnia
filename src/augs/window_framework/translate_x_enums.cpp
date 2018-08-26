#if BUILD_WINDOW_FRAMEWORK
#define XK_MISCELLANY
#define XK_LATIN1
#define XK_3270
#include <X11/keysymdef.h>

#include "augs/window_framework/translate_x_enums.h"

using namespace augs::event;
using namespace augs::event::keys;

key translate_keysym(const xcb_keysym_t m) {
	switch (m) {
	case XK_Caps_Lock:										return key::CAPSLOCK;

	case XK_Control_L:										return key::LCTRL;
	case XK_Control_R:										return key::RCTRL;

	case XK_Alt_L:											return key::LALT;
	case XK_Alt_R:											return key::RALT;

	case XK_Shift_L:										return key::LSHIFT;
	case XK_Shift_R:										return key::RSHIFT;

	case XK_Meta_L:											return key::LWIN;
	case XK_Meta_R:											return key::RWIN;

	case XK_Cancel:											return key::CANCEL;
	case XK_BackSpace:											return key::BACKSPACE;
	case XK_Tab:											return key::TAB;
	case XK_Clear:											return key::CLEAR;
	case XK_Return:											return key::ENTER;
	case XK_Pause:											return key::PAUSE;
	case XK_Escape:											return key::ESC;
	case XK_space:											return key::SPACE;
	case XK_Page_Up:											return key::PAGEUP;
	case XK_Page_Down:										return key::PAGEDOWN;
	case XK_End:											return key::END;
	case XK_Home:											return key::HOME;
	case XK_Left:											return key::LEFT;
	case XK_Up:												return key::UP;
	case XK_Right:											return key::RIGHT;
	case XK_Down:											return key::DOWN;
	case XK_Select:											return key::SELECT;
	case XK_Print:											return key::PRINT;
	case XK_Execute:										return key::EXECUTE;
	case XK_3270_PrintScreen:								return key::PRINTSCREEN;
	case XK_Insert:											return key::INSERT;
	case XK_Delete:											return key::DEL;
	case XK_Help:											return key::HELP;

	case XK_KP_Multiply:										return key::MULTIPLY;
	case XK_KP_Add:											return key::ADD;
	case XK_KP_Separator:										return key::SEPARATOR;
	case XK_KP_Subtract:										return key::SUBTRACT;
	case XK_KP_Decimal:										return key::DECIMAL;
	case XK_KP_Divide:											return key::DIVIDE;
															
	case XK_F1:												return key::F1;
	case XK_F2:												return key::F2;
	case XK_F3:												return key::F3;
	case XK_F4:												return key::F4;
	case XK_F5:												return key::F5;
	case XK_F6:												return key::F6;
	case XK_F7:												return key::F7;
	case XK_F8:												return key::F8;
	case XK_F9:												return key::F9;
	case XK_F10:											return key::F10;
	case XK_F11:											return key::F11;
	case XK_F12:											return key::F12;
	case XK_F13:											return key::F13;
	case XK_F14:											return key::F14;
	case XK_F15:											return key::F15;
	case XK_F16:											return key::F16;
	case XK_F17:											return key::F17;
	case XK_F18:											return key::F18;
	case XK_F19:											return key::F19;
	case XK_F20:											return key::F20;
	case XK_F21:											return key::F21;
	case XK_F22:											return key::F22;
	case XK_F23:											return key::F23;
	case XK_F24:											return key::F24;
	case XK_a:												return key::A;
	case XK_b:												return key::B;
	case XK_c:												return key::C;
	case XK_d:												return key::D;
	case XK_e:												return key::E;
	case XK_f:												return key::F;
	case XK_g:												return key::G;
	case XK_h:												return key::H;
	case XK_i:												return key::I;
	case XK_j:												return key::J;
	case XK_k:												return key::K;
	case XK_l:												return key::L;
	case XK_m:												return key::M;
	case XK_n:												return key::N;
	case XK_o:												return key::O;
	case XK_p:												return key::P;
	case XK_q:												return key::Q;
	case XK_r:												return key::R;
	case XK_s:												return key::S;
	case XK_t:												return key::T;
	case XK_u:												return key::U;
	case XK_v:												return key::V;
	case XK_w:												return key::W;
	case XK_x:												return key::X;
	case XK_y:												return key::Y;
	case XK_z:												return key::Z;
	case XK_0:												return key::_0;
	case XK_1:												return key::_1;
	case XK_2:												return key::_2;
	case XK_3:												return key::_3;
	case XK_4:												return key::_4;
	case XK_5:												return key::_5;
	case XK_6:												return key::_6;
	case XK_7:												return key::_7;
	case XK_8:												return key::_8;
	case XK_9:												return key::_9;
#if 0
	case XK_Volume_mute:									return key::VOLUME_MUTE;
	case XK_Volume_up:										return key::VOLUME_UP;
	case XK_Volume_down:									return key::VOLUME_DOWN;
	case XK_Media_next_track:								return key::NEXT_TRACK;
	case XK_Media_prev_track:								return key::PREV_TRACK;
	case XK_Media_stop:										return key::STOP_TRACK;
	case XK_Media_play_pause:								return key::PLAY_PAUSE_TRACK;

	case XK_Numlock:										return key::NUMLOCK;
	case XK_Scroll:											return key::SCROLL;

#endif
	case XK_equal:									return key::EQUAL;
	case XK_semicolon:											return key::SEMICOLON;
	case XK_plus:										return key::PLUS;

	case XK_comma:										return key::COMMA;
	case XK_minus:										return key::MINUS;
	case XK_period:										return key::PERIOD;
	case XK_slash:											return key::SLASH;
	case XK_grave:											return key::TILDE;
	case XK_bracketleft:											return key::OPEN_SQUARE_BRACKET;
	case XK_backslash:											return key::BACKSLASH;
	case XK_bracketright:											return key::CLOSE_SQUARE_BRACKET;
	case XK_apostrophe:											return key::APOSTROPHE;

	case XK_KP_Insert: return key::NUMPAD0;
	case XK_KP_End: return key::NUMPAD1;
	case XK_KP_Down: return key::NUMPAD2;
	case XK_KP_Next: return key::NUMPAD3;
	case XK_KP_Left: return key::NUMPAD4;
	case XK_KP_Begin: return key::NUMPAD5;
	case XK_KP_Right: return key::NUMPAD6;
	case XK_KP_Home: return key::NUMPAD7;
	case XK_KP_Up: return key::NUMPAD8;
	case XK_KP_Prior: return key::NUMPAD9;
	default:												return key::INVALID;
	}
}
#endif
