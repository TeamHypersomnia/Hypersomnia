#pragma once
#include <string>
#include <bitset>

#include "augs/math/vec2.h"
#include "augs/padding_byte.h"

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
				enum class key {
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
					EQUAL,
					VOLUME_MUTE,
					VOLUME_DOWN,
					VOLUME_UP,
					NEXT_TRACK,
					PREV_TRACK,
					STOP_TRACK,
					PLAY_PAUSE_TRACK,
					SEMICOLON,
					PLUS,
					COMMA,
					MINUS,
					PERIOD,
					SLASH,
					DASH,
					OPEN_SQUARE_BRACKET,
					BACKSLASH,
					CLOSE_SQUARE_BRACKET,
					APOSTROPHE,
					COUNT = 256,
				};

				bool is_numpad_key(const key);
				std::wstring key_to_wstring(const key);
				key wstring_to_key(const std::wstring&);
			}

			enum class key_change {
				NO_CHANGE,
				PRESSED,
				RELEASED
			};

			struct change {
				message msg = message::unknown;
				bool repeated = false;
				std::array<padding_byte, 2> pad;
				
				union {
					struct mouse_data {
						vec2t<short> rel;
					} mouse;

					struct scroll_data {
						int amount;
					} scroll;

					struct key_data {
						keys::key key;

						operator keys::key() const {
							return key;
						}
					} key;

					struct character_data {
						wchar_t utf16;
					} character;
				};

				change();

				bool operator==(const change&) const;

				key_change get_key_change() const;
				bool uses_mouse() const;
				bool was_any_key_pressed() const;
				bool was_any_key_released() const;
				bool was_key_pressed(const keys::key) const;
				bool was_key_released(const keys::key) const;
			};

			struct state {
				std::bitset<256> keys;

				struct mouse_info {
					vec2i pos;
					vec2i ldrag;
					vec2i rdrag;
				} mouse;

				vec2i screen_size;

				void unset_keys();
				void apply(const change&);
				bool get_mouse_key(const unsigned) const;
				bool is_set(const keys::key) const;
			};
		}
	}
}
