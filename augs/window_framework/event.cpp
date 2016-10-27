#include "event.h"
#include "augs/ensure.h"

namespace augs {
	namespace window {
		namespace event {
			change::change() {
				memset(this, 0, sizeof(change));
			}

			key_change change::get_key_change() const {
				switch (msg) {
				case message::ltripleclick: return key_change::PRESSED;
				case message::keydown: return key_change::PRESSED;
				case message::keyup:  return key_change::RELEASED;
				case message::ldoubleclick:  return key_change::PRESSED;
				case message::mdoubleclick: return key_change::PRESSED;
				case message::rdoubleclick: return key_change::PRESSED;
				case message::ldown: return key_change::PRESSED;
				case message::lup: return key_change::RELEASED;
				case message::mdown: return key_change::PRESSED;
				case message::mup: return key_change::RELEASED;
				case message::rdown: return key_change::PRESSED;
				case message::rup: return key_change::RELEASED;
				default: return key_change::NO_CHANGE; break;
				}
			}

			void state::apply(const change& dt) {
				const auto ch = dt.get_key_change();

				if (ch == key_change::PRESSED) {
					keys[dt.key.key] = true;
				}
				else if (ch == key_change::RELEASED) {
					keys[dt.key.key] = false;
				}
				else if (dt.msg == message::mousemotion) {
					mouse.pos += dt.mouse.rel;
					
					if (!get_mouse_key(0)) {
						mouse.ldrag.x = mouse.pos.x;
						mouse.ldrag.y = mouse.pos.y;
					}
					
					if (!get_mouse_key(1)) {
						mouse.rdrag.x = mouse.pos.x;
						mouse.rdrag.y = mouse.pos.y;
					}
				}
			}

			bool state::get_mouse_key(unsigned n) const {
				switch (n) {
				case 0: return keys.test(keys::LMOUSE);
				case 1: return keys.test(keys::RMOUSE);
				case 2: return keys.test(keys::MMOUSE);
				case 3: return keys.test(keys::MOUSE4);
				case 4: return keys.test(keys::MOUSE5);
				default: ensure(false); return false;
				}
			}

			void state::unset_keys() {
				keys.reset();
			}

			namespace keys {
				bool is_numpad_key(int k) {
					if(k >= NUMPAD0 && k <= NUMPAD9) return true;
					return false;
				}
			}
		}
	}
}