#pragma once

namespace augs {
	enum class filtering_type {
		// GEN INTROSPECTOR enum class augs::filtering_type
		NEAREST_NEIGHBOR,
		LINEAR,
		COUNT
		// END GEN INTROSPECTOR
	};

	struct renderer_settings {
		// GEN INTROSPECTOR struct augs::renderer_settings
		filtering_type default_filtering = filtering_type::NEAREST_NEIGHBOR;
		// END GEN INTROSPECTOR

		bool operator==(const renderer_settings& b) const = default;
	};
}
