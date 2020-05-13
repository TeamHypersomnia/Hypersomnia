#if BUILD_WINDOW_FRAMEWORK && USE_GLFW
#include <GLFW/glfw3.h>
#include "augs/window_framework/translate_glfw_enums.h"

using namespace augs::event;
using namespace augs::event::keys;

augs::event::keys::key translate_glfw_mouse_key(const int key) {
	switch (key) {
		case GLFW_MOUSE_BUTTON_LEFT: return key::LMOUSE;
		case GLFW_MOUSE_BUTTON_RIGHT: return key::RMOUSE;
		case GLFW_MOUSE_BUTTON_MIDDLE: return key::MMOUSE;

		case GLFW_MOUSE_BUTTON_4: return key::MOUSE4;
		case GLFW_MOUSE_BUTTON_5: return key::MOUSE5;
		default: return key::INVALID;
	}
}

key translate_glfw_key(const int m) {
	switch (m) {
	case GLFW_KEY_CAPS_LOCK:										return key::CAPSLOCK;

	case GLFW_KEY_LEFT_CONTROL:										return key::LCTRL;
	case GLFW_KEY_RIGHT_CONTROL:										return key::RCTRL;

	case GLFW_KEY_LEFT_ALT:											return key::LALT;
	case GLFW_KEY_RIGHT_ALT:											return key::RALT;

	case GLFW_KEY_LEFT_SHIFT:										return key::LSHIFT;
	case GLFW_KEY_RIGHT_SHIFT:										return key::RSHIFT;

	case GLFW_KEY_LEFT_SUPER:											return key::LWIN;
	case GLFW_KEY_RIGHT_SUPER:											return key::RWIN;

	case GLFW_KEY_BACKSPACE:											return key::BACKSPACE;
	case GLFW_KEY_TAB:											return key::TAB;
	case GLFW_KEY_ENTER:											return key::ENTER;
	case GLFW_KEY_PAUSE:											return key::PAUSE;
	case GLFW_KEY_ESCAPE:											return key::ESC;
	case GLFW_KEY_SPACE:											return key::SPACE;
	case GLFW_KEY_PAGE_UP:											return key::PAGEUP;
	case GLFW_KEY_PAGE_DOWN:										return key::PAGEDOWN;
	case GLFW_KEY_END:											return key::END;
	case GLFW_KEY_HOME:											return key::HOME;
	case GLFW_KEY_LEFT:											return key::LEFT;
	case GLFW_KEY_UP:												return key::UP;
	case GLFW_KEY_RIGHT:											return key::RIGHT;
	case GLFW_KEY_DOWN:											return key::DOWN;
	case GLFW_KEY_PRINT_SCREEN:								return key::PRINTSCREEN;
	case GLFW_KEY_INSERT:											return key::INSERT;
	case GLFW_KEY_DELETE:											return key::DEL;

	case GLFW_KEY_KP_MULTIPLY:										return key::MULTIPLY;
	case GLFW_KEY_KP_ADD:											return key::ADD;
	case GLFW_KEY_KP_SUBTRACT:										return key::SUBTRACT;
	case GLFW_KEY_KP_DECIMAL:										return key::DECIMAL;
	case GLFW_KEY_KP_DIVIDE:											return key::DIVIDE;
															
	case GLFW_KEY_F1:												return key::F1;
	case GLFW_KEY_F2:												return key::F2;
	case GLFW_KEY_F3:												return key::F3;
	case GLFW_KEY_F4:												return key::F4;
	case GLFW_KEY_F5:												return key::F5;
	case GLFW_KEY_F6:												return key::F6;
	case GLFW_KEY_F7:												return key::F7;
	case GLFW_KEY_F8:												return key::F8;
	case GLFW_KEY_F9:												return key::F9;
	case GLFW_KEY_F10:											return key::F10;
	case GLFW_KEY_F11:											return key::F11;
	case GLFW_KEY_F12:											return key::F12;
	case GLFW_KEY_F13:											return key::F13;
	case GLFW_KEY_F14:											return key::F14;
	case GLFW_KEY_F15:											return key::F15;
	case GLFW_KEY_F16:											return key::F16;
	case GLFW_KEY_F17:											return key::F17;
	case GLFW_KEY_F18:											return key::F18;
	case GLFW_KEY_F19:											return key::F19;
	case GLFW_KEY_F20:											return key::F20;
	case GLFW_KEY_F21:											return key::F21;
	case GLFW_KEY_F22:											return key::F22;
	case GLFW_KEY_F23:											return key::F23;
	case GLFW_KEY_F24:											return key::F24;
	case GLFW_KEY_A:												return key::A;
	case GLFW_KEY_B:												return key::B;
	case GLFW_KEY_C:												return key::C;
	case GLFW_KEY_D:												return key::D;
	case GLFW_KEY_E:												return key::E;
	case GLFW_KEY_F:												return key::F;
	case GLFW_KEY_G:												return key::G;
	case GLFW_KEY_H:												return key::H;
	case GLFW_KEY_I:												return key::I;
	case GLFW_KEY_J:												return key::J;
	case GLFW_KEY_K:												return key::K;
	case GLFW_KEY_L:												return key::L;
	case GLFW_KEY_M:												return key::M;
	case GLFW_KEY_N:												return key::N;
	case GLFW_KEY_O:												return key::O;
	case GLFW_KEY_P:												return key::P;
	case GLFW_KEY_Q:												return key::Q;
	case GLFW_KEY_R:												return key::R;
	case GLFW_KEY_S:												return key::S;
	case GLFW_KEY_T:												return key::T;
	case GLFW_KEY_U:												return key::U;
	case GLFW_KEY_V:												return key::V;
	case GLFW_KEY_W:												return key::W;
	case GLFW_KEY_X:												return key::X;
	case GLFW_KEY_Y:												return key::Y;
	case GLFW_KEY_Z:												return key::Z;
	case GLFW_KEY_0:												return key::_0;
	case GLFW_KEY_1:												return key::_1;
	case GLFW_KEY_2:												return key::_2;
	case GLFW_KEY_3:												return key::_3;
	case GLFW_KEY_4:												return key::_4;
	case GLFW_KEY_5:												return key::_5;
	case GLFW_KEY_6:												return key::_6;
	case GLFW_KEY_7:												return key::_7;
	case GLFW_KEY_8:												return key::_8;
	case GLFW_KEY_9:												return key::_9;

	case GLFW_KEY_EQUAL:									return key::EQUAL;
	case GLFW_KEY_SEMICOLON:											return key::SEMICOLON;

	case GLFW_KEY_COMMA:										return key::COMMA;
	case GLFW_KEY_MINUS:										return key::MINUS;
	case GLFW_KEY_PERIOD:										return key::PERIOD;
	case GLFW_KEY_SLASH:											return key::SLASH;
	case GLFW_KEY_GRAVE_ACCENT:											return key::TILDE;
	case GLFW_KEY_LEFT_BRACKET:											return key::OPEN_SQUARE_BRACKET;
	case GLFW_KEY_BACKSLASH:											return key::BACKSLASH;
	case GLFW_KEY_RIGHT_BRACKET:											return key::CLOSE_SQUARE_BRACKET;
	case GLFW_KEY_APOSTROPHE:											return key::APOSTROPHE;

	case GLFW_KEY_KP_0: return key::NUMPAD0;
	case GLFW_KEY_KP_1: return key::NUMPAD1;
	case GLFW_KEY_KP_2: return key::NUMPAD2;
	case GLFW_KEY_KP_3: return key::NUMPAD3;
	case GLFW_KEY_KP_4: return key::NUMPAD4;
	case GLFW_KEY_KP_5: return key::NUMPAD5;
	case GLFW_KEY_KP_6: return key::NUMPAD6;
	case GLFW_KEY_KP_7: return key::NUMPAD7;
	case GLFW_KEY_KP_8: return key::NUMPAD8;
	case GLFW_KEY_KP_9: return key::NUMPAD9;

	case GLFW_KEY_WORLD_1: return key::WORLD1;
	case GLFW_KEY_WORLD_2: return key::WORLD2;
	default: return key::INVALID;
	}
}
#endif
