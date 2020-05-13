#pragma once
#include <string>
#include <sstream>
#include <optional>

#include "augs/math/vec2.h"
#include "augs/misc/enum/enum_boolset.h"
#include "augs/pad_bytes.h"

namespace augs {
	namespace event {
		enum class key_change {
			NO_CHANGE,
			PRESSED,
			RELEASED
		};

		enum class message {
			INVALID,

			close,
			quit,
			move,
			resize,
			activate,
			deactivate,
			click_activate,
			minimize,
			maximize,
			restore,
			clipboard_change,
			syskeydown,
			syskeyup,
			keydown,
			keyup,
			character,
			unichar,
			mousemotion,
			wheel,
			ldoubleclick,
			rdoubleclick,
			mdoubleclick,
			ltripleclick,

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
				TILDE,
				OPEN_SQUARE_BRACKET,
				BACKSLASH,
				CLOSE_SQUARE_BRACKET,
				APOSTROPHE,
				WHEELUP,
				WHEELDOWN,
				WORLD1,
				WORLD2,

				COUNT = 256,
			};

			bool is_mouse_key(const key);
			bool is_numpad_key(const key);
			std::optional<int> get_number(const key);

			std::string key_to_alternative_char_representation(const key);
			std::string key_to_string(const key);
			std::string key_to_string_shortened(const key);
			key string_to_key(const std::string&);
		}

		struct change {
			struct mouse_data {
				basic_vec2<short> rel;
				basic_vec2<short> pos;
			};

			struct scroll_data {
				int amount;
			};

			struct key_data {
				keys::key key;
			};

			struct character_data {
				unsigned code_point;
			};

			union data_type {
				mouse_data mouse;
				scroll_data scroll;
				key_data key;
				character_data character;
				
				data_type() {}
			};

			message msg = message::INVALID;
			data_type data;
			uint32_t timestamp = 0;

			change();

			bool operator==(const change&) const;

			keys::key get_key() const;
			key_change get_key_change() const;
			bool uses_mouse() const;
			bool uses_keyboard() const;
			bool was_any_key_pressed() const;
			bool was_any_key_released() const;
			bool was_pressed(const keys::key) const;
			bool was_released(const keys::key) const;

			bool is_shortcut_key() const;

			bool is_exit_message() const;

			friend std::ostream& operator<<(std::ostream& out, const change& x);
		};

		struct state {
			enum_boolset<keys::key> keys;

			struct mouse_info {
				vec2i pos;
				vec2i ldrag;
				vec2i rdrag;
			} mouse;

			void reset_keys();
			state& apply(const change&);
			
			template <class C>
			state& apply(const C& changes) {
				for (const auto& e : changes) {
					apply(e);
				}

				return *this;
			}

			bool get_mouse_key(const unsigned) const;

			std::vector<change> generate_all_releasing_changes() const;
			std::vector<change> generate_key_releasing_changes() const;
			std::vector<change> generate_mouse_releasing_changes() const;

			bool operator[](const keys::key k) const {
				return keys.test(k);
			}
		};
	}
}
