#pragma once
#include "game/detail/visible_entities.h"
#include "game/detail/hand_fuse_logic.h"
#include "game/enums/use_button_query_result.h"

struct defuse_nearby_bomb_result {
	use_button_query_result result;

	entity_id bomb_subject;
	entity_id defusing_subject;

	bool defusing_already = false;

	bool success() const {
		return result == use_button_query_result::SUCCESS;
	}

	void perform(cosmos& cosm) const {
		cosm[bomb_subject].template get<components::hand_fuse>().character_now_defusing = defusing_subject;
	}
};

template <class E>
auto query_defusing_nearby_bomb(const E& subject) {
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

	defuse_nearby_bomb_result result;
	result.result = U::NONE_FOUND;

	entities.for_each<render_layer::PLANTED_BOMBS>(cosm, [&](const auto& bomb_handle) {
		bomb_handle.template dispatch_on_having_all<components::hand_fuse>([&](const auto typed_bomb) -> callback_result {
			const auto& fuse = typed_bomb.template get<components::hand_fuse>();
			const auto character_now_defusing = cosm[fuse.character_now_defusing];
			
			if (character_now_defusing.dead()) {
				const auto fuse_logic = stepless_fuse_logic_provider(typed_bomb);

				if (fuse_logic.defusing_character_in_range(subject)) {
					if (fuse_logic.defusing_conditions_fulfilled(subject)) {
						result.bomb_subject = typed_bomb;
						result.defusing_subject = subject;
						result.result = U::SUCCESS;

						return callback_result::ABORT;
					}
					else {
						result.result = U::IN_RANGE_BUT_CANT;
					}
				}
			}
			else {
				result.defusing_already = fuse.character_now_defusing == subject.get_id();
			}

			return callback_result::CONTINUE;
		});
	});

	return result;
}
