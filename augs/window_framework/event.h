#pragma once
#include "augs/math/vec2.h"
#ifdef PLATFORM_WINDOWS
#include <windows.h>
#endif
#include <bitset>
#include "augs/padding_byte.h"

#undef max
#undef min
namespace augs {
	namespace window {
		namespace event {
			enum class message : unsigned char {
				unknown,
				ltripleclick,
				close,
				move,
				activate,
				minimize,
				maximize,
				restore,
				clipboard_change,
				keydown,
				keyup,
				character,
				unichar,
				mousemotion,
				wheel,
				ldoubleclick,
				mdoubleclick,
				xdoubleclick,
				rdoubleclick,
				ldown,
				lup,
				mdown,
				mup,
				xdown,
				xup,
				rdown,
				rup,
				COUNT
			};

			namespace keys {
				extern bool is_numpad_key(int);
				enum key {
#ifdef PLATFORM_WINDOWS
					INVALID,
					LMOUSE = VK_LBUTTON,
					RMOUSE = VK_RBUTTON,
					MMOUSE = VK_MBUTTON,
					MOUSE4 = VK_XBUTTON1,
					MOUSE5 = VK_XBUTTON2,
					CANCEL = VK_CANCEL,
					BACKSPACE = VK_BACK,
					TAB = VK_TAB,
					CLEAR = VK_CLEAR,
					ENTER = VK_RETURN,
					SHIFT = VK_SHIFT,
					CTRL = VK_CONTROL,
					ALT = VK_MENU,
					PAUSE = VK_PAUSE,
					CAPSLOCK = VK_CAPITAL,
					ESC = VK_ESCAPE,
					SPACE = VK_SPACE,
					PAGEUP = VK_PRIOR,
					PAGEDOWN = VK_NEXT,
					END = VK_END,
					HOME = VK_HOME,
					LEFT = VK_LEFT,
					UP = VK_UP,
					RIGHT = VK_RIGHT,
					DOWN = VK_DOWN,
					SELECT = VK_SELECT,
					PRINT = VK_PRINT,
					EXECUTE = VK_EXECUTE,
					PRINTSCREEN = VK_SNAPSHOT,
					INSERT = VK_INSERT,
					DEL = VK_DELETE,
					HELP = VK_HELP,
					LWIN = VK_LWIN,
					RWIN = VK_RWIN,
					APPS = VK_APPS,
					SLEEP = VK_SLEEP,
					NUMPAD0 = VK_NUMPAD0,
					NUMPAD1 = VK_NUMPAD1,
					NUMPAD2 = VK_NUMPAD2,
					NUMPAD3 = VK_NUMPAD3,
					NUMPAD4 = VK_NUMPAD4,
					NUMPAD5 = VK_NUMPAD5,
					NUMPAD6 = VK_NUMPAD6,
					NUMPAD7 = VK_NUMPAD7,
					NUMPAD8 = VK_NUMPAD8,
					NUMPAD9 = VK_NUMPAD9,
					MULTIPLY = VK_MULTIPLY,
					ADD = VK_ADD,
					SEPARATOR = VK_SEPARATOR,
					SUBTRACT = VK_SUBTRACT,
					DECIMAL = VK_DECIMAL,
					DIVIDE = VK_DIVIDE,
					F1 = VK_F1,
					F2 = VK_F2,
					F3 = VK_F3,
					F4 = VK_F4,
					F5 = VK_F5,
					F6 = VK_F6,
					F7 = VK_F7,
					F8 = VK_F8,
					F9 = VK_F9,
					F10 = VK_F10,
					F11 = VK_F11,
					F12 = VK_F12,
					F13 = VK_F13,
					F14 = VK_F14,
					F15 = VK_F15,
					F16 = VK_F16,
					F17 = VK_F17,
					F18 = VK_F18,
					F19 = VK_F19,
					F20 = VK_F20,
					F21 = VK_F21,
					F22 = VK_F22,
					F23 = VK_F23,
					F24 = VK_F24,
					A = 'A',
					B = 'B',
					C = 'C',
					D = 'D',
					E = 'E',
					F = 'F',
					G = 'G',
					H = 'H',
					I = 'I',
					J = 'J',
					K = 'K',
					L = 'L',
					M = 'M',
					N = 'N',
					O = 'O',
					P = 'P',
					Q = 'Q',
					R = 'R',
					S = 'S',
					T = 'T',
					U = 'U',
					V = 'V',
					W = 'W',
					X = 'X',
					Y = 'Y',
					Z = 'Z',
					_0 = '0',
					_1 = '1',
					_2 = '2',
					_3 = '3',
					_4 = '4',
					_5 = '5',
					_6 = '6',
					_7 = '7',
					_8 = '8',
					_9 = '9',
					NUMLOCK = VK_NUMLOCK,
					SCROLL = VK_SCROLL,
					LSHIFT = VK_LSHIFT,
					RSHIFT = VK_RSHIFT,
					LCTRL = VK_LCONTROL,
					RCTRL = VK_RCONTROL,
					LALT = VK_LMENU,
					RALT = VK_RMENU,
					DASH = VK_OEM_3,
					COUNT = 256,
#elif PLATFORM_LINUX
					INVALID,
					LMOUSE,
					RMOUSE,
					MMOUSE,
					MOUSE4,
					MOUSE5,
					CANCEL,
					BACKSPACE,
					TAB,
					CLEAR,
					ENTER,
					SHIFT,
					CTRL,
					ALT,
					PAUSE,
					CAPSLOCK,
					ESC,
					SPACE,
					PAGEUP,
					PAGEDOWN,
					END,
					HOME,
					LEFT,
					UP,
					RIGHT,
					DOWN,
					SELECT,
					PRINT,
					EXECUTE,
					PRINTSCREEN,
					INSERT,
					DEL,
					HELP,
					LWIN,
					RWIN,
					APPS,
					SLEEP,
					NUMPAD0,
					NUMPAD1,
					NUMPAD2,
					NUMPAD3,
					NUMPAD4,
					NUMPAD5,
					NUMPAD6,
					NUMPAD7,
					NUMPAD8,
					NUMPAD9,
					MULTIPLY,
					ADD,
					SEPARATOR,
					SUBTRACT,
					DECIMAL,
					DIVIDE,
					F1,
					F2,
					F3,
					F4,
					F5,
					F6,
					F7,
					F8,
					F9,
					F10,
					F11,
					F12,
					F13,
					F14,
					F15,
					F16,
					F17,
					F18,
					F19,
					F20,
					F21,
					F22,
					F23,
					F24,
					A,
					B,
					C,
					D,
					E,
					F,
					G,
					H,
					I,
					J,
					K,
					L,
					M,
					N,
					O,
					P,
					Q,
					R,
					S,
					T,
					U,
					V,
					W,
					X,
					Y,
					Z,
					_0,
					_1,
					_2,
					_3,
					_4,
					_5,
					_6,
					_7,
					_8,
					_9,
					NUMLOCK,
					SCROLL,
					LSHIFT,
					RSHIFT,
					LCTRL,
					RCTRL,
					LALT,
					RALT,
					DASH,
#endif
				};
			}

			enum key_change {
				NO_CHANGE,
				PRESSED,
				RELEASED
			};

			struct change {
				message msg = message::unknown;
				bool repeated = false;
				padding_byte pad[2];
				
				union {
					struct mouse_data {
						vec2t<short> rel;
					} mouse;

					struct scroll_data {
						int amount;
					} scroll;

					struct key_data {
						keys::key key;
					} key;

					struct character_data {
						wchar_t utf16;
					} character;
				};

				change();

				key_change get_key_change() const;
			};

			struct state {
				std::bitset<256> keys;

				struct mouse_info {
					vec2i pos;
					vec2i ldrag;
					vec2i rdrag;
				} mouse;

				void unset_keys();
				void apply(const change&);
				bool get_mouse_key(unsigned) const;
			};
		}
	}
}
