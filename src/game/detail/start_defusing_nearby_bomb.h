#pragma once
#include "game/detail/visible_entities.h"
#include "game/detail/hand_fuse_logic.h"

template <class E>
bool start_defusing_nearby_bomb(const logic_step step, const E& subject) {
	auto& cosm = subject.get_cosmos();
	const auto max_defuse_radius = subject.template get<invariants::sentience>().max_defuse_radius;
	const auto where = subject.get_logic_transform().pos;

	auto& entities = thread_local_visible_entities();

	entities.reacquire_all_and_sort({
		cosm,
		camera_cone(camera_eye(where, 1.f), vec2i::square(max_defuse_radius)),
		visible_entities_query::accuracy_type::EXACT,
		render_layer_filter::whitelist(render_layer::PLANTED_BOMBS),
		{ { tree_of_npo_type::RENDERABLES } }
	});

	bool found_one = false;

	entities.for_each<render_layer::PLANTED_BOMBS>(cosm, [&](const auto typed_handle) {
		typed_handle.template dispatch_on_having_all<components::hand_fuse>([&](const auto typed_bomb) -> callback_result {
			auto& fuse = typed_bomb.template get<components::hand_fuse>();

			const auto character_now_defusing = cosm[fuse.character_now_defusing];
			
			if (character_now_defusing.dead()) {
				fuse.character_now_defusing = subject;

				const auto fuse_logic = fuse_logic_provider(typed_bomb, step);

				if (fuse_logic.defusing_conditions_fulfilled()) {
					found_one = true;
					return callback_result::ABORT;
				}
				else {
					fuse.character_now_defusing = {};
				}
			}

			return callback_result::CONTINUE;
		});
	});

	return found_one;
}

