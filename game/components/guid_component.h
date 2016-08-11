#pragma once
#include "game/build_settings.h"

namespace components {
	struct guid {
#if COSMOS_TRACKS_GUIDS
		unsigned value = 0;
#endif
	};
}