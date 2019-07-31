#pragma once
#include "augs/graphics/vertex.h"
#include "game/enums/particle_layer.h"

struct particle_triangle_buffers {
	per_particle_layer_t<augs::vertex_triangle_buffer> diffuse;
	augs::vertex_triangle_buffer neons;

	void clear() {
		for (auto& p : diffuse) {
			p.clear();
		}

		neons.clear();
	}
};

