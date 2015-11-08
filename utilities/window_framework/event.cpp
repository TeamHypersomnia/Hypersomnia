#pragma once
#include "event.h"

namespace augs {
	namespace window {
		namespace event {
			namespace keys {
				bool is_numpad_key(int k) {
					if(k >= NUMPAD0 && k <= NUMPAD9) return true;
					return false;
				}
			}
		}
	}
}