#include "game/transcendental/entity_handle.h"

#include "application/intercosm.h"
#include "application/setups/editor/editor_command_input.h"
#include "application/setups/editor/editor_folder.h"
#include "application/setups/editor/gui/editor_entity_selector.h"

#include "application/setups/editor/commands/duplicate_entities_command.h"
#include "application/setups/editor/gui/find_aabb_of.h"
#include "application/setups/editor/detail/editor_transform_utils.h"

std::string duplicate_entities_command::describe() const {
	return built_description;
}

void duplicate_entities_command::push_entry(const const_entity_handle handle) {
	handle.dispatch([&](const auto typed_handle) {
		using E = entity_type_of<decltype(typed_handle)>;
		using vector_type = make_data_vector<E>;

		duplicated_entities.get<vector_type>().push_back({ typed_handle.get_id() });
	});
}

bool duplicate_entities_command::empty() const {
	return size() == 0;
}

void duplicate_entities_command::redo(const editor_command_input in) {
	in.purge_selections();
	in.interrupt_tweakers();

	auto& f = in.folder;
	auto& selections = f.view.selected_entities;

	auto& cosm = in.get_cosmos();

	const bool does_mirroring = mirror_direction.non_zero();

	auto duplicate = [&](auto&& new_transform_setter) {
		duplicated_entities.for_each([&](auto& e) {
			const auto duplicated = cosmic::specific_clone_entity(cosm[e.source_id]);
			new_transform_setter(duplicated);
			e.duplicated_id = duplicated.get_id();

			selections.emplace(e.duplicated_id);

			if (does_mirroring) {
				selections.emplace(e.source_id);
			}
		});
	};

	if (does_mirroring) {
		auto duplicate_with_flip = [&](auto transformation, const bool hori, const bool vert) {
			duplicate([&](const auto typed_handle) {
				if (const auto source_transform = typed_handle.find_logic_transform()) {
					if (typed_handle.has_independent_transform()) {
						const auto mirrored_transform = transformation(typed_handle.get_logic_transform());
						fix_pixel_imperfections(mirrored_transform);
						typed_handle.set_logic_transform(mirrored_transform);

						if (const auto sprite = typed_handle.template find<components::sprite>()) {
							if (hori) {
								auto& f = sprite->flip.horizontally;
								f = !f;
							}

							if (vert) {
								auto& f = sprite->flip.vertically;
								f = !f;
							}
						}
					}
				}
			});
		};

		const auto& selected_entities = in.folder.view.selected_entities;

		auto for_each_source_subject = [this](auto callback){ 
			duplicated_entities.for_each([&](auto& e) {
				callback(e.source_id);
			});
		};

		if (const auto source_aabb = find_aabb_of(cosm, for_each_source_subject)) {
			const auto target_aabb = *source_aabb + mirror_direction * source_aabb->get_size();

			if (mirror_direction == vec2i(1, 0)) {
				const auto vertical_axis_x = source_aabb->r;

				duplicate_with_flip(
					[vertical_axis_x](const components::transform source) {
						const auto new_rotation = vec2::from_degrees(source.rotation).negate_y().degrees();

						const auto dist_to_axis = vertical_axis_x - source.pos.x;
						const auto new_pos = source.pos + vec2(dist_to_axis, 0.f) * 2;

						return components::transform(new_pos, new_rotation);
					},
					true,
					false
				);
			}

			if (mirror_direction == vec2i(-1, 0)) {
				const auto vertical_axis_x = source_aabb->l;

				duplicate_with_flip(
					[vertical_axis_x](const components::transform source) {
						const auto new_rotation = vec2::from_degrees(source.rotation).negate_y().degrees();

						const auto dist_to_axis = source.pos.x - vertical_axis_x;
						const auto new_pos = source.pos - vec2(dist_to_axis, 0.f) * 2;

						return components::transform(new_pos, new_rotation);
					},
					true,
					false
				);
			}

			if (mirror_direction == vec2i(0, 1)) {
				const auto horizontal_axis_y = source_aabb->b;

				duplicate_with_flip(
					[horizontal_axis_y](const components::transform source) {
						const auto new_rotation = vec2::from_degrees(source.rotation).negate_y().degrees();

						const auto dist_to_axis = horizontal_axis_y - source.pos.y;
						const auto new_pos = source.pos + vec2(0.f, dist_to_axis) * 2;

						return components::transform(new_pos, new_rotation);
					},
					false,
					true
				);
			}

			if (mirror_direction == vec2i(0, -1)) {
				const auto horizontal_axis_y = source_aabb->t;

				duplicate_with_flip(
					[horizontal_axis_y](const components::transform source) {
						const auto new_rotation = vec2::from_degrees(source.rotation).negate_y().degrees();

						const auto dist_to_axis = source.pos.y - horizontal_axis_y;
						const auto new_pos = source.pos - vec2(0.f, dist_to_axis) * 2;

						return components::transform(new_pos, new_rotation);
					},
					false,
					true
				);
			}
		}
	}
	else {
		/* Standard duplication in-place. Editor will usually initiate the move command immediately. */
		selections.clear();
		duplicate([](auto&&...) {});
	}

	cosmic::reinfer_all_entities(cosm);
}

void duplicate_entities_command::undo(const editor_command_input in) const {
	in.purge_selections();
	in.interrupt_tweakers();

	auto& cosm = in.get_cosmos();

	auto& f = in.folder;
	auto& selections = f.view.selected_entities;

	duplicated_entities.for_each_reverse([&](const auto& e) {
		cosmic::undo_last_create_entity(cosm[e.duplicated_id]);
		selections.emplace(e.source_id);
	});

	/* 
		At this point, some audiovisual systems might have dead ids with valid indirectors.
		However, the real_index fields inside relevant indirectors will be correctly set to -1,
		indicating that the entity is dead.

		After creating another entity via a different method than a redo of the just redone command,
		it might so happen that the audiovisual systems start pointing to a completely unrelated entity.
		
		We could fix this by always incrementing the id versions on creating via redoing,
		but the same problem will nevertheless persist in networked environments.
	*/
}
