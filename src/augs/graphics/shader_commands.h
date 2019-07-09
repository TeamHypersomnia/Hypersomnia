#pragma once

using GLint = int;

namespace augs {
	namespace graphics {
		using uniform_payload_variant = std::variant<
			vec2,
			vec2i,
			rgba,
			rgba::rgb_type,
			vec3,
			vec4,
			float,
			int
		>;

		struct set_uniform_command {
			GLint uniform_id;
			uniform_payload_variant payload;
		};

		struct set_projection_command {
			std::array<float, 16> payload;
		};
	}
}
