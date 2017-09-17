#pragma once
#include "augs/math/camera_cone.h"

#include "game/enums/render_layer.h"
#include "game/transcendental/entity_id.h"

struct visible_entities_query {
	const cosmos& cosm;
	const camera_cone cone;
};

struct visible_entities {
	using id_type = entity_id;
	
	using per_layer_type = per_render_layer_t<std::vector<id_type>>;
	using all_type = std::vector<id_type>;

	all_type all;
	per_layer_type per_layer;

	visible_entities() = default;

	visible_entities(const visible_entities_query);
	visible_entities& operator=(const visible_entities&) = delete;

	/*
		This function will be used instead of copy-assignment operator,
		in order to take advantage of the reserved space in containers.
	*/
	void reacquire(const visible_entities_query);
	void clear_dead(const cosmos&);
};