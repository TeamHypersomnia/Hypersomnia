#include "game/cosmos/entity_handle.h"
#include "game/detail/inventory/inventory_slot_handle.h"
#include "game/cosmos/cosmic_functions.h"

#include "application/intercosm.h"
#include "application/setups/debugger/debugger_command_input.h"
#include "application/setups/debugger/debugger_folder.h"
#include "application/setups/debugger/gui/debugger_entity_selector.h"

#include "application/setups/debugger/commands/duplicate_entities_command.h"
#include "view/rendering_scripts/find_aabb_of.h"
#include "application/setups/debugger/detail/debugger_transform_utils.h"
#include "application/setups/debugger/commands/debugger_command_sanitizer.h"
#include "application/setups/debugger/debugger_selection_groups.hpp"

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

void duplicate_entities_command::redo(const debugger_command_input in) {
	auto access = allocate_new_entity_access();

	clear_undo_state();

	in.purge_selections();
	in.interrupt_tweakers();

	auto& f = in.folder;
	auto& selections = f.commanded->view_ids.selected_entities;
	(void)selections;
	auto& groups = f.commanded->view_ids.selection_groups;

	auto& cosm = in.get_cosmos();

	const bool does_mirroring = mirror_direction.is_nonzero();

	const auto new_group_suffix = [this](){
		if (mirror_direction.is_zero()) {
			return "-dup";
		}	
		if (mirror_direction == vec2i(1, 0)) {
			return "-right";
		}
		if (mirror_direction == vec2i(-1, 0)) {
			return "-left";
		}
		if (mirror_direction == vec2i(0, -1)) {
			return "-up";
		}
		if (mirror_direction == vec2i(0, 1)) {
			return "-down";
		}

		return "-INVALID";
	}();

	const auto free_untitled_group = groups.get_free_group_name(std::string("Ungrouped") + new_group_suffix + "-%x");

	auto duplicate = [&](auto&& new_transform_setter) {
		duplicated_entities.for_each([&](auto& e) {
			try {
				const auto duplicated = cosmic::specific_clone_entity(access, *cosm[e.source_id]);

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

				if (in.settings.keep_source_entities_selected_on_mirroring) {
					if (does_mirroring) {
						selections.emplace(e.source_id);
					}
				}
			}
			catch (const entity_creation_error&) {

			}
		});

		// FIXME: Parametrize this?
		//created_grouping.redo(in);
	};

	if (does_mirroring) {
		auto duplicate_with_flip = [&](auto calc_mirror_offset, const bool hori, const bool vert) {
			duplicate([&](const auto typed_handle) {
				if (const auto ir = typed_handle.find_independent_transform()) {
					{
						const auto source_transform = *ir;
						const auto new_rotation = source_transform.get_direction().neg_y().degrees();

						const auto mirror_offset = calc_mirror_offset(
							source_transform.pos, 
							typed_handle.find_aabb()
						);

						auto mirrored_transform = transformr(mirror_offset + source_transform.pos, new_rotation);
						fix_pixel_imperfections(mirrored_transform);

						flip_flags flip;
						flip.horizontally = hori;
						flip.vertically = vert;

						if (!typed_handle.do_flip(flip)) {
							if (flip.horizontally) {
								mirrored_transform.rotation += 180;
							}
						}

						typed_handle.set_logic_transform(mirrored_transform);
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
			const auto mir_dir = mirror_direction;

			auto calc_mirror_offset = [source_aabb, mir_dir](const transformr& source, const std::optional<ltrb>& aabb) {
				if (mir_dir == vec2i(1, 0)) {
					if (aabb) {
						return vec2(2 * source_aabb->r - aabb->l - aabb->r, 0.f);
					}
					else {
						const auto dist_to_axis = source_aabb->r - source.pos.x;
						return vec2(dist_to_axis * 2, 0.f);
					}
				}

				else if (mir_dir == vec2i(-1, 0)) {
					if (aabb) {
						return vec2(2 * source_aabb->l - aabb->l - aabb->r, 0.f);
					}
					else {
						const auto dist_to_axis = source.pos.x - source_aabb->l;
						return vec2(-(dist_to_axis * 2), 0.f);
					}
				}

				else if (mir_dir == vec2i(0, 1)) {
					if (aabb) {
						return vec2(0.f, 2 * source_aabb->b - aabb->t - aabb->b);
					}
					else {
						const auto dist_to_axis = source_aabb->b - source.pos.y;
						return vec2(0.f, dist_to_axis * 2);
					}
				}

				else {
					// (mir_dir == vec2i(0, -1)) 
					if (aabb) {
						return vec2(0.f, 2 * source_aabb->t - aabb->t - aabb->b);
					}
					else {
						const auto dist_to_axis = source.pos.y - source_aabb->t;
						return vec2(0.f, -(dist_to_axis * 2));
					}
				}
			};

			if (mirror_direction == vec2i(1, 0)) {
				duplicate_with_flip(calc_mirror_offset, true, false);
			}

			if (mirror_direction == vec2i(-1, 0)) {
				duplicate_with_flip(calc_mirror_offset, true, false);
			}

			if (mirror_direction == vec2i(0, 1)) {
				duplicate_with_flip(calc_mirror_offset, false, true);
			}

			if (mirror_direction == vec2i(0, -1)) {
				duplicate_with_flip(calc_mirror_offset, false, true);
			}
		}
	}
	else {
		/* 
			Standard duplication in-place. 
			Editor initiates the move command immediately. 
		*/
		duplicate([](auto&&...) {});
	}

	cosmic::reinfer_all_entities(cosm);
}

void duplicate_entities_command::undo(const debugger_command_input in) {
	// FIXME: Parametrize this?
	//created_grouping.undo(in);

	in.purge_selections();
	in.interrupt_tweakers();

	auto& cosm = in.get_cosmos();

	auto& f = in.folder;
	auto& selections = f.commanded->view_ids.selected_entities;

	duplicated_entities.for_each_reverse([&](const auto& e) {
		cosmic::undo_last_create_entity(cosm[e.duplicated_id]);
		selections.emplace(e.source_id);
	});

	{
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

	clear_undo_state();
}

void duplicate_entities_command::sanitize(const debugger_command_input in) {
	created_grouping.sanitize(in);

	sanitize_affected_entities(in, duplicated_entities, [](const auto& entry) {
		return entry.source_id;
	});
}

void duplicate_entities_command::clear_undo_state() {
	created_grouping.clear_undo_state();
}
