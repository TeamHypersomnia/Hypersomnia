#include "event.h"
#include "augs/ensure.h"
#include "augs/templates/traits/triviality_traits.h"
#include "augs/templates/container_templates.h"

namespace augs {
	namespace event {
		std::ostream& operator<<(std::ostream& out, const augs::event::change& x) {
			if (x.was_any_key_pressed()) {
				return out << "Pressed: " << augs::event::keys::key_to_string(x.data.key.key);
			}

			if (x.was_any_key_released()) {
				return out << "Released: " << augs::event::keys::key_to_string(x.data.key.key);
			}

			if (x.msg == message::mousemotion) {
				return out << "Mouse moved to: " << x.data.mouse.pos << ", Rel: " << x.data.mouse.rel;
			}

			return out;
		}

		change::change() {
			std::memset(this, 0, sizeof(change));
			msg = message::INVALID;
		}

		keys::key change::get_key() const {
			using namespace keys;

			switch (msg) {
			case message::ltripleclick: return key::LMOUSE;
			case message::syskeydown: return data.key.key;
			case message::syskeyup: return data.key.key;
			case message::keydown: return data.key.key;
			case message::keyup: return data.key.key;
			case message::ldoubleclick:  return key::LMOUSE;
			case message::mdoubleclick: return key::MMOUSE;
			case message::rdoubleclick: return key::RMOUSE;
			default: return key::INVALID;
			}
		}

		key_change change::get_key_change() const {
			switch (msg) {
			case message::ltripleclick: return key_change::PRESSED;
			case message::keydown: return key_change::PRESSED;
			case message::keyup:  return key_change::RELEASED;
			case message::syskeydown: return key_change::PRESSED;
			case message::syskeyup:  return key_change::RELEASED;
			case message::ldoubleclick:  return key_change::PRESSED;
			case message::mdoubleclick: return key_change::PRESSED;
			case message::rdoubleclick: return key_change::PRESSED;
			default: return key_change::NO_CHANGE; break;
			}
		}

		bool change::operator==(const change& c) const {
			return trivial_compare(*this, c);
		}

		bool change::uses_mouse() const {
			switch (msg) {
			case message::mousemotion: return true;
			case message::ltripleclick: return true;
			case message::ldoubleclick:  return true;
			case message::mdoubleclick: return true;
			case message::rdoubleclick: return true;
			case message::keydown: return is_mouse_key(get_key());
			case message::keyup: return is_mouse_key(get_key());
			default: return false; break;
			}
		}
		
		bool change::uses_keyboard() const {
			switch (msg) {
			case message::keydown: return !is_mouse_key(get_key());
			case message::keyup: return !is_mouse_key(get_key());
			case message::syskeydown: return true;
			case message::syskeyup: return true;
			case message::character: return true;
			case message::unichar: return true;
			default: return false;
			}
		}

		bool change::was_any_key_pressed() const {
			return get_key_change() == key_change::PRESSED;
		}

		bool change::was_any_key_released() const {
			return get_key_change() == key_change::RELEASED;
		}

		bool change::was_pressed(const keys::key k) const {
			return was_any_key_pressed() && get_key() == k;
		}

		bool change::was_released(const keys::key k) const {
			return was_any_key_released() && get_key() == k;
		}

		bool change::is_exit_message() const {
			return msg == message::close
				|| msg == message::quit
				|| (msg == message::syskeydown && get_key() == keys::key::F4)
			;
		}

		bool change::is_shortcut_key() const {
			if (get_key_change() != key_change::NO_CHANGE) {
				/* Let shortcut keys propagate */
				switch (get_key()) {
					case keys::key::LCTRL: return true;
					case keys::key::LALT: return true;
					case keys::key::LSHIFT: return true;
					case keys::key::RCTRL: return true;
					case keys::key::RALT: return true;
					case keys::key::RSHIFT: return true;
					case keys::key::CTRL: return true;
					case keys::key::ALT: return true;
					case keys::key::SHIFT: return true;
					default: return false;
				}
			}

			return false;
		}

		state& state::apply(const change& dt) {
			const auto ch = dt.get_key_change();

			if (ch == key_change::PRESSED) {
				keys.set(dt.get_key(), true);
			}
			else if (ch == key_change::RELEASED) {
				keys.set(dt.get_key(), false);
			}
			else if (dt.msg == message::mousemotion) {
				mouse.pos = dt.data.mouse.pos;
				
				if (!get_mouse_key(0)) {
					mouse.ldrag.x = mouse.pos.x;
					mouse.ldrag.y = mouse.pos.y;
				}

				if (!get_mouse_key(1)) {
					mouse.rdrag.x = mouse.pos.x;
					mouse.rdrag.y = mouse.pos.y;
				}
			}

			return *this;
		}

		bool state::get_mouse_key(const unsigned n) const {
			switch (n) {
			case 0: return keys.test(keys::key::LMOUSE);
			case 1: return keys.test(keys::key::RMOUSE);
			case 2: return keys.test(keys::key::MMOUSE);
			default: ensure(false); return false;
			}
		}

		std::vector<change> state::generate_all_releasing_changes() const {
			auto all = generate_key_releasing_changes();
			concatenate(all, generate_mouse_releasing_changes());
			return all;
		}

		std::vector<change> state::generate_key_releasing_changes() const {
			std::vector<change> output;

			for (std::size_t i = 0; i < keys.size(); ++i) {
				const auto k = static_cast<keys::key>(i);

				if (is_mouse_key(k)) {
					continue;
				}

				if (keys[k]) {
					change c;
					c.data.key.key = k;
					c.msg = message::keyup;
					output.push_back(c);
				}
			}

			return output;
		}

		std::vector<change> state::generate_mouse_releasing_changes() const {
			std::vector<change> output;

			for (std::size_t i = 0; i < keys.size(); ++i) {
				const auto k = static_cast<keys::key>(i);

				if (!is_mouse_key(k)) {
					continue;
				}

				if (keys[k]) {
					change c;
					c.data.key.key = k;
					c.msg = message::keyup;
					output.push_back(c);
				}
			}

			return output;
		}

		void state::reset_keys() {
			keys.reset();
		}

		namespace keys {
			bool is_numpad_key(const key k) {
				if (static_cast<int>(k) >= static_cast<int>(key::NUMPAD0) && static_cast<int>(k) <= static_cast<int>(key::NUMPAD9)) return true;
				return false;
			}

			std::optional<int> get_number(const key k) {
				const auto left = int(key::_0);
				const auto right = int(key::_9);
				const auto v = int(k);

				if (v >= left && v <= right) {
					return v - left;
				}

				return std::nullopt;
			}

			bool is_mouse_key(const key k) {
				switch(k) {
					case key::LMOUSE: return true;
					case key::RMOUSE: return true;
					case key::MMOUSE: return true;
					case key::MOUSE4: return true;
					case key::MOUSE5: return true;
					case key::WHEELUP: return true;
					case key::WHEELDOWN: return true;
					default: return false;
				}
			}

			std::string key_to_string(const key k) {
				switch (k) {
				case key::INVALID: return "INVALID"; break;
				case key::LMOUSE: return "Left Mouse Button"; break;
				case key::RMOUSE: return "Right Mouse Button"; break;
				case key::MMOUSE: return "Middle Mouse Button"; break;
				case key::MOUSE4: return "Mouse Button 4"; break;
				case key::MOUSE5: return "Mouse Button 5"; break;
				case key::CANCEL: return "Cancel"; break;
				case key::BACKSPACE: return "Backspace"; break;
				case key::TAB: return "Tab"; break;
				case key::CLEAR: return "Clear"; break;
				case key::ENTER: return "Enter"; break;
				case key::SHIFT: return "Shift"; break;
				case key::CTRL: return "Control"; break;
				case key::ALT: return "Alt"; break;
				case key::PAUSE: return "Pause"; break;
				case key::CAPSLOCK: return "Caps Lock"; break;
				case key::ESC: return "Escape"; break;
				case key::SPACE: return "Space"; break;
				case key::PAGEUP: return "Page Up"; break;
				case key::PAGEDOWN: return "Page Down"; break;
				case key::END: return "End"; break;
				case key::HOME: return "Home"; break;
				case key::LEFT: return "Left Arrow"; break;
				case key::UP: return "Up Arrow"; break;
				case key::RIGHT: return "Right Arrow"; break;
				case key::DOWN: return "Down Arrow"; break;
				case key::SELECT: return "Select"; break;
				case key::PRINT: return "Print"; break;
				case key::EXECUTE: return "Execute"; break;
				case key::PRINTSCREEN: return "Print Screen"; break;
				case key::INSERT: return "Insert"; break;
				case key::DEL: return "Del"; break;
				case key::HELP: return "Help"; break;
				case key::LWIN: return "Left Windows Key"; break;
				case key::RWIN: return "Right Windows Key"; break;
				case key::APPS: return "Apps"; break;
				case key::SLEEP: return "Sleep"; break;
				case key::NUMPAD0: return "Numpad 0"; break;
				case key::NUMPAD1: return "Numpad 1"; break;
				case key::NUMPAD2: return "Numpad 2"; break;
				case key::NUMPAD3: return "Numpad 3"; break;
				case key::NUMPAD4: return "Numpad 4"; break;
				case key::NUMPAD5: return "Numpad 5"; break;
				case key::NUMPAD6: return "Numpad 6"; break;
				case key::NUMPAD7: return "Numpad 7"; break;
				case key::NUMPAD8: return "Numpad 8"; break;
				case key::NUMPAD9: return "Numpad 9"; break;
				case key::MULTIPLY: return "Multiply"; break;
				case key::ADD: return "Add"; break;
				case key::SEPARATOR: return "Separator"; break;
				case key::SUBTRACT: return "Subtract"; break;
				case key::DECIMAL: return "Decimal"; break;
				case key::DIVIDE: return "Divide"; break;
				case key::F1: return "F1"; break;
				case key::F2: return "F2"; break;
				case key::F3: return "F3"; break;
				case key::F4: return "F4"; break;
				case key::F5: return "F5"; break;
				case key::F6: return "F6"; break;
				case key::F7: return "F7"; break;
				case key::F8: return "F8"; break;
				case key::F9: return "F9"; break;
				case key::F10: return "F10"; break;
				case key::F11: return "F11"; break;
				case key::F12: return "F12"; break;
				case key::F13: return "F13"; break;
				case key::F14: return "F14"; break;
				case key::F15: return "F15"; break;
				case key::F16: return "F16"; break;
				case key::F17: return "F17"; break;
				case key::F18: return "F18"; break;
				case key::F19: return "F19"; break;
				case key::F20: return "F20"; break;
				case key::F21: return "F21"; break;
				case key::F22: return "F22"; break;
				case key::F23: return "F23"; break;
				case key::F24: return "F24"; break;
				case key::A: return "A"; break;
				case key::B: return "B"; break;
				case key::C: return "C"; break;
				case key::D: return "D"; break;
				case key::E: return "E"; break;
				case key::F: return "F"; break;
				case key::G: return "G"; break;
				case key::H: return "H"; break;
				case key::I: return "I"; break;
				case key::J: return "J"; break;
				case key::K: return "K"; break;
				case key::L: return "L"; break;
				case key::M: return "M"; break;
				case key::N: return "N"; break;
				case key::O: return "O"; break;
				case key::P: return "P"; break;
				case key::Q: return "Q"; break;
				case key::R: return "R"; break;
				case key::S: return "S"; break;
				case key::T: return "T"; break;
				case key::U: return "U"; break;
				case key::V: return "V"; break;
				case key::W: return "W"; break;
				case key::X: return "X"; break;
				case key::Y: return "Y"; break;
				case key::Z: return "Z"; break;
				case key::_0: return "0"; break;
				case key::_1: return "1"; break;
				case key::_2: return "2"; break;
				case key::_3: return "3"; break;
				case key::_4: return "4"; break;
				case key::_5: return "5"; break;
				case key::_6: return "6"; break;
				case key::_7: return "7"; break;
				case key::_8: return "8"; break;
				case key::_9: return "9"; break;
				case key::NUMLOCK: return "Num Lock"; break;
				case key::SCROLL: return "Scroll"; break;
				case key::LSHIFT: return "Left Shift"; break;
				case key::RSHIFT: return "Right Shift"; break;
				case key::LCTRL: return "Left Control"; break;
				case key::RCTRL: return "Right Control"; break;
				case key::LALT: return "Left Alt"; break;
				case key::RALT: return "Right Alt"; break;
				case key::TILDE: return "Tilde"; break;
				case key::EQUAL: return "Equal"; break;
				case key::VOLUME_MUTE: return "Volume Mute"; break;
				case key::VOLUME_DOWN: return "Volume Down"; break;
				case key::VOLUME_UP: return "Volume Up"; break;
				case key::NEXT_TRACK: return "Next Track"; break;
				case key::PREV_TRACK: return "Prev Track"; break;
				case key::STOP_TRACK: return "Stop Track"; break;
				case key::PLAY_PAUSE_TRACK: return "Play Or Pause"; break;
				case key::SEMICOLON: return "Semicolon"; break;
				case key::PLUS: return "Plus"; break;
				case key::COMMA: return "Comma"; break;
				case key::MINUS: return "Minus"; break;
				case key::PERIOD: return "Period"; break;
				case key::SLASH: return "Slash"; break;
				case key::OPEN_SQUARE_BRACKET: return "Open Square Bracket"; break;
				case key::BACKSLASH: return "Backslash"; break;
				case key::CLOSE_SQUARE_BRACKET: return "Close Square Bracket"; break;
				case key::APOSTROPHE: return "Apostrophe"; break;
				case key::WHEELUP: return "Wheel Up"; break;
				case key::WHEELDOWN: return "Wheel Down"; break;
				default: return "Invalid key"; break;
				}
			}

			key string_to_key(const std::string& str) {
				for (key i = key::INVALID; i < key::COUNT; i = key(int(i) + 1)) {
					if (key_to_string(i) == str) {
						return i;
					}
				}

				return key::INVALID;
			}
		}
	}
}
