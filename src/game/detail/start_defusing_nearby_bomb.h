#pragma once
#include "game/detail/visible_entities.h"
#include "game/detail/hand_fuse_logic.h"
#include "game/enums/use_button_query_result.h"

template <class E>
auto start_defusing_nearby_bomb(const logic_step step, const E& subject) {
	auto& cosm = subject.get_cosmos();
	const auto max_defuse_radius = subject.template get<invariants::sentience>().use_button_radius;
	const auto where = subject.get_logic_transform().pos;

	auto& entities = thread_local_visible_entities();

	entities.reacquire_all_and_sort({
		cosm,
		camera_cone(camera_eye(where, 1.f), vec2i::square(max_defuse_radius * 2)),
		visible_entities_query::accuracy_type::EXACT,
		render_layer_filter::whitelist(render_layer::PLANTED_BOMBS),
		{ { tree_of_npo_type::RENDERABLES } }
	});

	using U = use_button_query_result;
	auto result = U::NONE_FOUND;

	entities.for_each<render_layer::PLANTED_BOMBS>(cosm, [&](const auto typed_handle) {
		typed_handle.template dispatch_on_having_all<components::hand_fuse>([&](const auto typed_bomb) -> callback_result {
			auto& fuse = typed_bomb.template get<components::hand_fuse>();

			const auto character_now_defusing = cosm[fuse.character_now_defusing];
			
			if (character_now_defusing.dead()) {
				fuse.character_now_defusing = subject;

				const auto fuse_logic = fuse_logic_provider(typed_bomb, step);

				if (fuse_logic.defusing_character_in_range()) {
					if (fuse_logic.defusing_conditions_fulfilled()) {
						result = U::SUCCESS;
						return callback_result::ABORT;
					}
					else {
						result = U::IN_RANGE_BUT_CANT;
					}
				}

				fuse.character_now_defusing = {};
			}

			return callback_result::CONTINUE;
		});
	});

	return result;
}

