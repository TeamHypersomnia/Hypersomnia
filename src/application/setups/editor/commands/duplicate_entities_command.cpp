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
		duplicated_entities.get_for<E>().push_back({ typed_handle.get_id(), {} });
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
	auto& groups = f.view.selection_groups;

	auto& cosm = in.get_cosmos();

	const bool does_mirroring = mirror_direction.non_zero();

	const auto new_group_suffix = [this](){
		if (mirror_direction.is_zero()) {
			return "-dup";
		}	
		if (mirror_direction == vec2i(1, 0)) {
			return "-mr";
		}
		if (mirror_direction == vec2i(-1, 0)) {
			return "-ml";
		}
		if (mirror_direction == vec2i(0, -1)) {
			return "-mu";
		}
		if (mirror_direction == vec2i(0, 1)) {
			return "-md";
		}

		return "-INVALID";
	}();

	const auto free_untitled_group = groups.get_free_group_name(std::string("Ungrouped") + new_group_suffix + "-%x");

	auto duplicate = [&](auto&& new_transform_setter) {
		duplicated_entities.for_each([&](auto& e) {
			const auto duplicated = cosmic::specific_clone_entity(cosm[e.source_id]);

			const bool group_found = groups.on_group_entry_of(e.source_id, [&](auto, const auto& group, auto) {
				/* 
					If the source entity was in a group,
				   	move the duplicated one to a corresponding group for duplicates.
				*/

				const auto new_group_name = group.name + new_group_suffix;
				const auto new_idx = groups.get_group_by(new_group_name);

				created_grouping.push_entry(duplicated);
				created_grouping.group_indices_after.push_back(new_idx);
			});

			if (!group_found) {
				created_grouping.push_entry(duplicated);

				const auto new_idx = groups.get_group_by(free_untitled_group);
				created_grouping.group_indices_after.push_back(new_idx);
			}

			new_transform_setter(duplicated);
			e.duplicated_id = duplicated.get_id();

			selections.emplace(e.duplicated_id);

			if (does_mirroring) {
				selections.emplace(e.source_id);
			}
		});

		created_grouping.redo(in);
	};

	if (does_mirroring) {
		auto duplicate_with_flip = [&](auto calc_mirror_offset, const bool hori, const bool vert) {
			duplicate([&](const auto typed_handle) {
				if (const auto source_transform = typed_handle.find_logic_transform()) {
					if (typed_handle.has_independent_transform()) {
						{
							const auto source_transform = typed_handle.get_logic_transform();
							const auto new_rotation = vec2::from_degrees(source_transform.rotation).negate_y().degrees();

							const auto mirror_offset = calc_mirror_offset(
								source_transform.pos, 
								typed_handle.find_aabb()
							);

							auto mirrored_transform = transformr(mirror_offset + source_transform.pos, new_rotation);
							fix_pixel_imperfections(mirrored_transform);
							typed_handle.set_logic_transform(mirrored_transform);
						}

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

		auto for_each_source_subject = [this](auto callback){ 
			duplicated_entities.for_each([&](auto& e) {
				callback(e.source_id);
			});
		};

		if (const auto source_aabb = find_aabb_of(cosm, for_each_source_subject)) {
			if (mirror_direction == vec2i(1, 0)) {
				const auto vertical_axis_x = source_aabb->r;

				duplicate_with_flip(
					[vertical_axis_x](const transformr& source, const std::optional<ltrb>& aabb) {
						if (aabb) {
							const auto dist_from_right_to_axis = vertical_axis_x - aabb->r;
							return vec2(aabb->w() + dist_from_right_to_axis * 2, 0.f);
						}
						else {
							const auto dist_to_axis = vertical_axis_x - source.pos.x;
							return vec2(dist_to_axis * 2, 0.f);
						}
					},
					true,
					false
				);
			}

			if (mirror_direction == vec2i(-1, 0)) {
				const auto vertical_axis_x = source_aabb->l;

				duplicate_with_flip(
					[vertical_axis_x](const transformr& source, const std::optional<ltrb>& aabb) {
						if (aabb) {
							const auto dist_from_left_to_axis = aabb->l - vertical_axis_x;
							return vec2(-(aabb->w() + dist_from_left_to_axis * 2), 0.f);
						}
						else {
							const auto dist_to_axis = source.pos.x - vertical_axis_x;
							return vec2(-(dist_to_axis * 2), 0.f);
						}
					},
					true,
					false
				);
			}

			if (mirror_direction == vec2i(0, 1)) {
				const auto horizontal_axis_y = source_aabb->b;
				LOG_NVPS(horizontal_axis_y);

				duplicate_with_flip(
					[horizontal_axis_y](const transformr& source, const std::optional<ltrb>& aabb) {
						if (aabb) {
							const auto dist_from_bottom_to_axis = horizontal_axis_y - aabb->b;
							return vec2(0.f, aabb->h() + dist_from_bottom_to_axis * 2);
						}
						else {
							const auto dist_to_axis = horizontal_axis_y - source.pos.y;
							return vec2(0.f, dist_to_axis * 2);
						}
					},
					false,
					true
				);
			}

			if (mirror_direction == vec2i(0, -1)) {
				const auto horizontal_axis_y = source_aabb->t;

				duplicate_with_flip(
					[horizontal_axis_y](const transformr& source, const std::optional<ltrb>& aabb) {
						if (aabb) {
							const auto dist_from_top_to_axis = aabb->t - horizontal_axis_y;
							return vec2(0.f, -(aabb->h() + dist_from_top_to_axis * 2));
						}
						else {
							const auto dist_to_axis = source.pos.y - horizontal_axis_y;
							return vec2(0.f, -(dist_to_axis * 2));
						}
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

void duplicate_entities_command::undo(const editor_command_input in) {
	created_grouping.undo(in);

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
