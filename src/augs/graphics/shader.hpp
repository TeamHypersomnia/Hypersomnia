#pragma once
#include "augs/graphics/shader.h"
#include "augs/graphics/renderer.h"
#include "augs/graphics/common_uniform_name.h"

namespace augs {
	namespace graphics {
		template <class... Args>
		void shader_program::set_uniform(renderer& r, const common_uniform_name name, Args&&... args) const {
			r.push_object_command(*this, set_uniform_command { uniform_map[name], std::forward<Args>(args)... });
		}
	}
}
