#pragma once
#include <array>

#include "game/transcendental/types_specification/all_messages_declaration.h"
#include "augs/misc/delta.h"
#include "game/transcendental/cosmic_entropy.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/detail/state_for_drawing_camera.h"
#include "game/enums/render_layer.h"

#include "augs/entity_system/storage_for_message_queues.h"

class cosmos;
struct immediate_hud;
struct aabb_highlighter;

namespace augs {
	class renderer;
}

class viewing_step {
public:
	viewing_step(const cosmos&, const immediate_hud& hud, aabb_highlighter&, const augs::variable_delta&, augs::renderer&, state_for_drawing_camera camera_state);

	state_for_drawing_camera camera_state;

	const cosmos& cosm;
	const immediate_hud& hud;
	aabb_highlighter& world_hover_highlighter;
	augs::variable_delta delta;
	augs::renderer& renderer;

	augs::variable_delta get_delta() const;

	vec2 get_screen_space(vec2 pos) const;
	const cosmos& get_cosmos() const;

	std::vector<const_entity_handle> visible_entities;
	std::array<std::vector<const_entity_handle>, render_layer::LAYER_COUNT> visible_per_layer;
};

class fixed_step {
	friend class cosmos;
	fixed_step(cosmos&, cosmic_entropy);

public:
	storage_for_all_message_queues messages;
	cosmos& cosm;
	cosmic_entropy entropy;

	augs::fixed_delta get_delta() const;
	cosmos& get_cosmos() const;
};