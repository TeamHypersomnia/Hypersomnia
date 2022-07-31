#include "game/cosmos/entity_handle.h"
#include "application/setups/editor/editor_setup.h"
#include "application/setups/editor/editor_setup.hpp"
#include "game/detail/inventory/inventory_slot_handle.h"

#include "application/setups/editor/commands/duplicate_nodes_command.h"
#include "view/rendering_scripts/find_aabb_of.h"
#include "application/setups/editor/detail/editor_transform_utils.h"

#include "augs/misc/pool/pool_allocate.h"

#include "augs/templates/traits/has_rotation.h"
#include "augs/templates/traits/has_flip.h"

template <class F>
std::optional<ltrb> editor_setup::find_aabb_of_typed_nodes(F&& for_each_typed_node) const {
	ltrb total;

	for_each_typed_node(
		[&](const auto id) {
			if (const auto node = find_node(id)) {
				// TODO: In case of prefabs, this should consider *all* entity ids, not just the scene_entity_id.

				if (const auto handle = scene.world[node->scene_entity_id]) {
					if (const auto aabb = find_aabb_of_entity(handle)) {
						total.contain(*aabb);
					}
				}
			}
		}
	);

	if (total.good()) {
		return total;
	}

	return std::nullopt;
}

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

std::string duplicate_nodes_command::describe() const {
	return built_description;
}

void duplicate_nodes_command::push_entry(const editor_node_id id) {
	id.type_id.dispatch([&]<typename E>(const E) {
		duplicated_nodes.get_for<E>().push_back({ editor_typed_node_id<E>::from_generic(id), {} });
	});
}

bool duplicate_nodes_command::empty() const {
	return size() == 0;
}

void duplicate_nodes_command::redo(const editor_command_input in) {
	clear_undo_state();

	in.setup.clear_inspector();

	const bool does_mirroring = mirror_direction.is_nonzero();

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

	auto duplicate = [&](auto&& new_transform_setter) {
		duplicated_nodes.for_each([&]<typename T>(T& e) {
			using N = typename T::node_type;

			auto& pool = in.setup.project.nodes.get_pool_for<N>();

			const auto& old_node = pool.get(e.source_id.raw);
			const auto old_name = old_node.unique_name;
			const auto new_name = in.setup.get_free_node_name_for(old_name + new_group_suffix);

			const auto [duplicated_id, duplicated_node] = pool.allocate(old_node);
			e.duplicated_id.set(duplicated_id);
			duplicated_node.unique_name = new_name;

			new_transform_setter(duplicated_node);

			const auto source_id_generic = e.source_id.operator editor_node_id();
			const auto duplicated_id_generic = e.duplicated_id.operator editor_node_id();

			in.setup.register_node_in_layer(duplicated_id_generic, source_id_generic);
			in.setup.inspect_add_quiet(duplicated_id_generic);
		});
	};

	if (does_mirroring) {
		auto duplicate_with_flip = [&](auto calc_mirror_offset, const bool hori, const bool vert) {
			duplicate([&](auto& duplicated_node) {
				auto& e = duplicated_node.editable;
				const auto source_transform = get_node_transform(e);

				{
					const auto new_rotation = source_transform.get_direction().neg_y().degrees();

					const auto mirror_offset = calc_mirror_offset(
						source_transform.pos
					);

					auto mirrored_transform = transformr(mirror_offset + source_transform.pos, new_rotation);
					fix_pixel_imperfections(mirrored_transform);

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
			duplicated_nodes.for_each([&](auto& e) {
				callback(e.source_id);
			});
		};

		if (const auto source_aabb = in.setup.find_aabb_of_typed_nodes(for_each_source_node_id)) {
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

	in.setup.after_quietly_adding_inspected();
}

void duplicate_nodes_command::undo(const editor_command_input in) {
	in.setup.clear_inspector();

	duplicated_nodes.for_each_reverse([&]<typename T>(T& entry) {
		using N = typename T::node_type;

		const auto source_id = entry.source_id.operator editor_node_id();
		const auto duplicated_id = entry.duplicated_id.operator editor_node_id();

		in.setup.unregister_node_from_layer(duplicated_id);

		auto& pool = in.setup.project.nodes.get_pool_for<N>();
		pool.undo_last_allocate(entry.duplicated_id.raw);

		in.setup.inspect_add_quiet(source_id);
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

	in.setup.after_quietly_adding_inspected();

	clear_undo_state();
}

void duplicate_nodes_command::clear_undo_state() {

}
