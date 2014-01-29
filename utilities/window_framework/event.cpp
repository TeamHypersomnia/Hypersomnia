#pragma once
#include "stdafx.h"
#include "event.h"

namespace augmentations {
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