#include "game/cosmos/entity_handle.h"
#include "application/setups/editor/editor_setup.h"
#include "application/setups/editor/editor_setup.hpp"
#include "game/detail/inventory/inventory_slot_handle.h"

#include "application/setups/editor/commands/clone_nodes_command.h"
#include "application/setups/editor/detail/editor_transform_utils.h"

#include "augs/misc/pool/pool_allocate.h"

#include "augs/templates/traits/has_rotation.h"
#include "augs/templates/traits/has_flip.h"
#include "application/setups/editor/editor_setup_find_aabb_of_nodes.hpp"

template <class N>
transformr get_node_transform(const N& node) {
	auto rotation = 0.0f;

	if constexpr(has_rotation_v<N>) {
		rotation = node.rotation;
	}

	return transformr(node.pos, rotation);
}

template <class N>
void set_node_transform(N& node, const transformr& new_transform) {
	node.pos = new_transform.pos;

	if constexpr(has_rotation_v<N>) {
		node.rotation = new_transform.rotation;
	}
}

std::string clone_nodes_command::describe() const {
	return built_description;
}

void clone_nodes_command::push_entry(const editor_node_id id) {
	cloned_nodes.push_back({ id, {} });
}

bool clone_nodes_command::empty() const {
	return size() == 0;
}

std::vector<editor_node_id> clone_nodes_command::get_all_cloned() const { 
	std::vector<editor_node_id> result;

	for (const auto e : cloned_nodes) {
		result.push_back(e.cloned_id);
	}

	return result;
}

static auto peel_clone_suffix(std::string s) {
	const auto orig = s;

	if (s.size() > 0 && s.back() == ')') {
		s.pop_back();

		s = cut_trailing_number(s);

		if (ends_with(s, " (")) {
			s.pop_back();
			s.pop_back();

			return s;
		}
	}

	return orig;
}

void clone_nodes_command::redo(const editor_command_input in) {
	clear_undo_state();

	if (!omit_inspector) {
		in.setup.clear_inspector();
	}

	const bool does_mirroring = mirror_direction.is_nonzero();

	const auto new_group_suffix = [this](){
		if (mirror_direction.is_zero()) {
			return "";
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

	auto duplicate = [&](auto&& new_transform_setter) {
		for (auto& e : cloned_nodes) {
			e.source_id.type_id.dispatch([&]<typename T>(const T) {
				auto& pool = in.setup.project.nodes.get_pool_for<T>();
				auto old_node = pool.get(e.source_id.raw);
				const auto new_name = in.setup.get_free_node_name_for(peel_clone_suffix(old_node.unique_name + new_group_suffix));
				const auto [cloned_id, cloned_node] = pool.allocate(std::move(old_node));

				editor_typed_node_id<T> new_id;
				new_id.set(cloned_id);
				e.cloned_id = new_id.operator editor_node_id();

				cloned_node.unique_name = new_name;
				cloned_node.chronological_order = in.setup.project.nodes.next_chronological_order++;
				cloned_node.scene_entity_id.unset();
				cloned_node.clear_cloned_fields();

				new_transform_setter(cloned_node);
			});

			const auto source_id_generic = e.source_id;
			const auto cloned_id_generic = e.cloned_id;

			auto register_in_layer = [&]() {
				if (target_new_layer.has_value()) {
					return in.setup.register_node_in_layer(cloned_id_generic, *target_new_layer, static_cast<std::size_t>(-1));
				}

				if (target_unified_location) {
					return in.setup.register_node_in_layer(cloned_id_generic, target_unified_location->first, target_unified_location->second);
				}

				return in.setup.register_node_in_layer(cloned_id_generic, source_id_generic);
			};

			const bool registered_in_layer = register_in_layer();
			ensure(registered_in_layer);
			(void)registered_in_layer;

			if (!omit_inspector) {
				in.setup.inspect_add_quiet(cloned_id_generic);
			}

			if (in.settings.keep_source_nodes_selected_on_mirroring) {
				if (does_mirroring) {
					in.setup.inspect_add_quiet(source_id_generic);
				}
			}
		};
	};

	if (does_mirroring) {
		auto clone_with_flip = [&](auto calc_mirror_offset, const bool hori, const bool vert) {
			duplicate([&]<typename N>(N& cloned_node) {
				auto& e = cloned_node.editable;
				const auto source_transform = get_node_transform(e);

				{
					const auto new_rotation = source_transform.get_direction().neg_y().degrees();

					const auto mirror_offset = calc_mirror_offset(
						source_transform.pos
					);

					auto mirrored_transform = transformr(mirror_offset + source_transform.pos, new_rotation);
					fix_pixel_imperfections(mirrored_transform);

					/*
						We need to account for nodes that do not have fields for flipping,
						like spawnpoints for example.

						Areas and points get special treatment when calculating flipped rotations,
						because they do not support flip flags.
					*/

					if constexpr(is_one_of_v<N, editor_prefab_node, editor_area_marker_node>) {
						if (vert) {
							mirrored_transform.rotation += 180;
						}
					}
					else {
						if constexpr(!has_flip_v<decltype(e)>) {
							if (hori) {
								mirrored_transform.rotation += 180;
							}
						}
					}

					set_node_transform(e, mirrored_transform);
				}

				if constexpr(has_flip_v<decltype(e)>) {
					if (hori) {
						e.flip_horizontally = !e.flip_horizontally;
					}

					if (vert) {
						e.flip_vertically = !e.flip_vertically;
					}
				}
			});
		};

		auto for_each_source_node_id = [this](auto callback){ 
			for (const auto& e : cloned_nodes) {
				callback(e.source_id);
			}
		};

		const auto source_aabb = [&]() {
			if (custom_aabb) {
				return custom_aabb;
			}

			return in.setup.find_aabb_of_nodes(for_each_source_node_id);
		}();

		if (source_aabb) {
			const auto mir_dir = mirror_direction;

			auto calc_mirror_offset = [source_aabb, mir_dir](const transformr& source) {
				if (mir_dir == vec2i(1, 0)) {
					const auto dist_to_axis = source_aabb->r - source.pos.x;
					return vec2(dist_to_axis * 2, 0.f);
				}

				else if (mir_dir == vec2i(-1, 0)) {
					const auto dist_to_axis = source.pos.x - source_aabb->l;
					return vec2(-(dist_to_axis * 2), 0.f);
				}

				else if (mir_dir == vec2i(0, 1)) {
					const auto dist_to_axis = source_aabb->b - source.pos.y;
					return vec2(0.f, dist_to_axis * 2);
				}
				else {
					const auto dist_to_axis = source.pos.y - source_aabb->t;
					return vec2(0.f, -(dist_to_axis * 2));
				}
			};

			if (mirror_direction == vec2i(1, 0)) {
				clone_with_flip(calc_mirror_offset, true, false);
			}

			if (mirror_direction == vec2i(-1, 0)) {
				clone_with_flip(calc_mirror_offset, true, false);
			}

			if (mirror_direction == vec2i(0, 1)) {
				clone_with_flip(calc_mirror_offset, false, true);
			}

			if (mirror_direction == vec2i(0, -1)) {
				clone_with_flip(calc_mirror_offset, false, true);
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

	if (!omit_inspector) {
		in.setup.after_quietly_adding_inspected();
	}
}

void clone_nodes_command::reverse_order() {
	reverse_range(cloned_nodes);
}

void clone_nodes_command::undo(const editor_command_input in) {
	if (!omit_inspector) {
		in.setup.clear_inspector();
	}

	for (auto& entry : reverse(cloned_nodes)) {
		const auto source_id = entry.source_id;
		const auto cloned_id = entry.cloned_id;

		in.setup.unregister_node_from_layer(cloned_id);

		cloned_id.type_id.dispatch([&]<typename T>(const T) {
			auto& pool = in.setup.project.nodes.get_pool_for<T>();
			pool.undo_last_allocate(cloned_id.raw);
		});

		if (!omit_inspector) {
			in.setup.inspect_add_quiet(source_id);
		}
	}

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

	if (!omit_inspector) {
		in.setup.after_quietly_adding_inspected();
	}

	clear_undo_state();
}

void clone_nodes_command::clear_undo_state() {

}
