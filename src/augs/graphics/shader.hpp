#pragma once
#include "augs/graphics/shader.h"
#include "augs/graphics/renderer.h"

namespace augs {
	namespace graphics {
		template <class... Args>
		void shader_program::set_uniform(renderer& r, const char* name, Args&&... args) const {
			r.push_object_command(*this, set_uniform_command { name, std::forward<Args>(args)... });
		}
	}
}
