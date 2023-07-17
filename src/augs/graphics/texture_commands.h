#pragma once
#include "augs/math/vec2.h"
#include "augs/graphics/renderer_settings.h"

namespace augs {
	namespace graphics {
		struct texImage2D_command {
			image source;
		};

		struct set_filtering_command {
			filtering_type type;
		};
	}
}
