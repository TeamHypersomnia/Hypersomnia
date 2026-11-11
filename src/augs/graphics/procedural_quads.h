#pragma once
#include <array>
#include <cstdint>
#include <variant>
#include <vector>
#include "augs/math/vec2.h"
#include "augs/graphics/rgba.h"
#include "augs/graphics/shader_commands.h"

namespace augs {
	struct procedural_quads_draw {
		uint32_t after_triangles = 0;
		uint32_t quad_count = 0;
		std::vector<graphics::set_uniform_command> uniforms_before;
		std::vector<graphics::set_uniform_command> uniforms_after;
		GLint vec4_array_uniform_id = -1;
		std::vector<vec4> vec4_array;
	};
}
