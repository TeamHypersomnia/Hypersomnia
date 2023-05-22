#include "game/cosmos/cosmos_solvable_access.h"
#include "game/cosmos/cosmic_functions.h"
#include "game/detail/inventory/inventory_slot_handle.h"
#include "application/intercosm.h"
#include "application/setups/editor/commands/node_transform_commands.h"
#include "view/rendering_scripts/find_aabb_of.h"
#include "game/inferred_caches/organism_cache.hpp"

#include "application/setups/editor/editor_setup.hpp"
#include "application/setups/editor/detail/editor_transform_utils.h"

//#include "application/setups/debugger/debugger_folder.h"
//#include "application/setups/debugger/editor_command_input.h"
//#include "application/setups/debugger/gui/debugger_entity_selector.h"

#include "augs/readwrite/memory_stream.h"
#include "augs/readwrite/byte_readwrite.h"
#include "game/cosmos/entity_handle.h"
#include "augs/templates/traits/has_rotation.h"
#include "application/setups/editor/editor_setup_for_each_inspected_entity.hpp"
#include "augs/templates/traits/has_size.h"
#include "augs/templates/traits/has_flip.h"

using active_resized_edges = resize_nodes_command::active_edges;
using delta_type = move_nodes_command::delta_type;
using moved_entities_type = move_nodes_command::moved_entities_type;
using resized_entities_type = resize_nodes_command::resized_entities_type;

template <class T>
static void reselect(T& entities, editor_setup& setup, current_selections_type& selections) {
	selections.clear();

	entities.for_each([&](const auto id) {
		selections.emplace(id);
	});

	setup.inspect_from_selector_state();
}

static void read_back_to_nodes(editor_setup& setup) {
	auto& cosm = setup.get_viewed_cosmos();

	bool rebuild_arena = false;

	setup.for_each_inspected_entity(
		[&](const entity_id id) {
			const auto handle = cosm[id];

			if (handle.dead()) {
				return;
			}

			const auto transform = handle.dispatch([](const auto& typed) { return typed.find_independent_transform(); });

			const auto size = handle.dispatch([](const auto& typed) -> std::optional<vec2i> { 
				if (const auto geo = typed.get().template find<components::overridden_geo>()) {
					if (geo->size.is_enabled) {
						return geo->size.value;
					}
				}

				return std::nullopt;
			});

			const auto flip = handle.dispatch([](const auto& typed) -> flip_flags { 
				if (const auto geo = typed.get().template find<components::overridden_geo>()) {
					return geo->flip;
				}

				return flip_flags();
			});

			if (transform == std::nullopt) {
				return;
			}

			auto read_back_to = [transform, size, flip]<typename N>(N& node, auto) {
				node.editable.pos = transform->pos;

				using E = decltype(node.editable);

				if constexpr(has_rotation_v<E>) {
					node.editable.rotation = transform->rotation;
				}

				if constexpr(has_size_v<E>) {
					if (size == std::nullopt) {
						node.editable.size.reset();
					}
					else {
						node.editable.size = *size;
					}
				}

				if constexpr(has_flip_v<E>) {
					node.editable.flip_horizontally = flip.horizontally;
					node.editable.flip_vertically = flip.vertically;
				}
			};

			setup.on_node(
				setup.to_node_id(id),
				[&]<typename N>(N& node, const auto node_id) {
					read_back_to(node, node_id);

					if constexpr(std::is_same_v<N, editor_prefab_node>) {
						rebuild_arena = true;
					}
				}
			);
		}
	);

	if (rebuild_arena) {
		setup.rebuild_arena();
	}
}


static void save_transforms(
	cosmos& cosm,
	const moved_entities_type& subjects,
	augs::ref_memory_stream& into
) {
	on_each_independent_transform(
		cosm,
		subjects,
		[&into](const auto& tr, auto&) {
			augs::write_bytes(into, tr);
		}
	);
}

static void unmove_entities(
	const cosmos_solvable_access key,
	cosmos& cosm,
	const moved_entities_type& subjects,
	augs::cref_memory_stream& from
) {
	on_each_independent_transform(
		cosm,
		subjects,
		[&from](auto& tr, auto&) {
			augs::read_bytes(from, tr);
		},
		key
	);
}

void save_sizes(
	cosmos& cosm,
	const resized_entities_type& subjects,
	augs::ref_memory_stream& into
) {
	subjects.for_each(
		[&](const auto& i) {
			const auto typed_handle = cosm[i];

			if constexpr(typed_handle.template has<components::overridden_geo>()) {
				const auto& overridden_geo = typed_handle.get().template get<components::overridden_geo>();

				augs::write_bytes(into, overridden_geo.size);
			}
		}
	);
}

void unresize_entities(
	const cosmos_solvable_access key,
	cosmos& cosm,
	const resized_entities_type& subjects,
	augs::cref_memory_stream& from
) {
	subjects.for_each(
		[&](const auto& i) {
			const auto typed_handle = cosm[i];

			if constexpr(typed_handle.template has<components::overridden_geo>()) {
				auto& overridden_geo = typed_handle.get(key).template get<components::overridden_geo>();

				augs::read_bytes(from, overridden_geo.size);
			}
		}
	);
}

static void move_entities(
	const cosmos_solvable_access key,
	cosmos& cosm,
	const moved_entities_type& subjects,
	const delta_type& dt,
	const std::optional<vec2> rotation_center,
	const special_entity_mover_op special
) {
	if (special == special_entity_mover_op::RESET_ROTATION) {
		auto rotation_resetter = 
			[&](auto& tr, auto&) {
				using T = remove_cref<decltype(tr)>;

				if constexpr(std::is_same_v<T, physics_engine_transforms>) {
					auto new_transform = tr.get();
					new_transform.rotation = 0;
					tr.set(new_transform);
				}
				else if constexpr(std::is_same_v<T, transformr>) {
					tr.rotation = 0;
				}
				else if constexpr(std::is_same_v<T, vec2>) {

				}
				else {
					static_assert(always_false_v<T>, "Unknown transform type.");
				}
			}
		;

		on_each_independent_transform(cosm, subjects, rotation_resetter, key);
	}

	const auto si = cosm.get_si();
	const auto dt_si = dt.to_si_space(si);

	if (rotation_center) {
		const auto center = *rotation_center;
		const auto center_meters = si.get_meters(center);

		auto rotator = 
			[&](auto& tr, auto&) {
				using T = remove_cref<decltype(tr)>;

				if constexpr(std::is_same_v<T, physics_engine_transforms>) {
					auto new_transform = tr.get();
					new_transform.rotate_radians_with_90_multiples(dt_si.rotation, center_meters);

					fix_pixel_imperfections(new_transform, si);

					tr.set(new_transform);
				}
				else if constexpr(std::is_same_v<T, transformr>) {
					tr.rotate_degrees_with_90_multiples(dt.rotation, center);
					fix_pixel_imperfections(tr);
				}
				else if constexpr(std::is_same_v<T, vec2>) {
					auto t = transformr(tr, 0);
					t.rotate_degrees_with_90_multiples(dt.rotation, center);
					fix_pixel_imperfections(t);
					tr = t.pos;
				}
				else {
					static_assert(always_false_v<T>, "Unknown transform type.");
				}
			}
		;

		on_each_independent_transform(cosm, subjects, rotator, key);
	}
	else {
		auto mover = 
			[&](auto& tr, auto&) {
				using T = remove_cref<decltype(tr)>;

				if constexpr(std::is_same_v<T, physics_engine_transforms>) {
					auto total_tr = tr.get();
					total_tr += dt_si;

					if (augs::is_epsilon(total_tr.rotation, AUGS_EPSILON<real32>)) {
						total_tr.pos = si.get_meters(vec2(si.get_pixels(total_tr.pos)).round_fract());
						total_tr.rotation = 0.f;
					}

					tr.set(total_tr);
				}
				else if constexpr(std::is_same_v<T, transformr>) {
					tr += dt;

					if (augs::is_epsilon(tr.rotation, AUGS_EPSILON<real32>)) {
						tr.pos.round_fract();
						tr.rotation = 0.f;
					}
				}
				else if constexpr(std::is_same_v<T, vec2>) {
					tr += dt.pos;
					tr.round_fract();
				}
				else {
					static_assert(always_false_v<T>, "Unknown transform type.");
				}
			}
		;

		on_each_independent_transform(cosm, subjects, mover, key);
	}
}

void resize_entities(
	const cosmos_solvable_access key,
	cosmos& cosm,
	const resized_entities_type& subjects,
	const resize_target_point& target_point,
	const active_resized_edges& edges,
	const bool both_axes_simultaneously
) {
	const auto si = cosm.get_si();

	const auto& world_resize_target_point = target_point.snapped;

	subjects.for_each(
		[&](const auto& i) {
			const auto typed_handle = cosm[i];

			const auto size_unit = [&]() {
				std::optional<vec2i> result;

				if constexpr(typed_handle.template has<invariants::sprite>()) {
					const auto& spr = typed_handle.template get<invariants::sprite>();

					if (spr.tile_excess_size) { 
						result = spr.size;
					}
				}

				if (target_point.used_grid.has_value()) {
					const auto unit = target_point.used_grid->unit_pixels;

					if (result.has_value()) {
						result->x = std::max(result->x, unit);
						result->y = std::max(result->y, unit);
					}
					else {
						result = vec2i::square(unit);
					}
				}

				return result;
			}();

			if constexpr(typed_handle.template has<components::overridden_geo>()) {
				typed_handle.access_independent_transform(
					[&](auto& transform) {
						using T = remove_cref<decltype(transform)>;

						const auto original_transform = [si, transform]() {
							if constexpr(std::is_same_v<T, physics_engine_transforms>) {
								return transform.get(si);
							}
							else if constexpr(std::is_same_v<T, transformr>) {
								(void)si;
								return transform;
							}
							else if constexpr(std::is_same_v<T, vec2>) {
								(void)si;
								return transformr(transform, 0);
							}
							else {
								static_assert(always_false_v<T>, "Unknown transform type.");
								return transformr();
							}
						}();

						const auto pos = original_transform.pos;
						const auto rot = original_transform.rotation;
						const auto current_size = typed_handle.get_logical_size();

						const auto ref = vec2(world_resize_target_point).rotate(-rot, pos);
						const auto rect = ltrb::center_and_size(pos, current_size);

						auto set_size = [&](const vec2 new_size) {
							auto& overridden_geo = typed_handle.get(key).template get<components::overridden_geo>();

							if (size_unit.has_value()) {
								vec2i s = new_size;

								if (edges.horizontal()) {
									s.x /= size_unit.value().x;
									s.x = std::max(s.x, 1);
									s.x *= size_unit.value().x;
								}

								if (edges.vertical()) {
									s.y /= size_unit.value().y;
									s.y = std::max(s.y, 1);
									s.y *= size_unit.value().y;
								}

								overridden_geo.size.emplace(s);
							}
							else {
								overridden_geo.size.emplace(new_size);
							}
						};

						auto set_pos = [&](const vec2 new_pos) {
							if constexpr(std::is_same_v<T, physics_engine_transforms>) {
								auto t = transform.get();
								t.pos = si.get_meters(new_pos);

								if (augs::is_epsilon(t.rotation, AUGS_EPSILON<real32>)) {
									t.pos = si.get_meters(vec2(si.get_pixels(t.pos)).round_fract());
									t.rotation = 0.f;
								}

								transform.set(t);
							}
							else if constexpr(std::is_same_v<T, transformr>) {
								transform.pos = new_pos;

								if (augs::is_epsilon(transform.rotation, AUGS_EPSILON<real32>)) {
									transform.pos.round_fract();
									transform.rotation = 0.f;
								}
							}
							else if constexpr(std::is_same_v<T, vec2>) {
								transform = new_pos;
								transform.round_fract();
							}
							else {
								static_assert(always_false_v<T>);
							}
						};

						vec2 desired_size = current_size;
						vec2 desired_pos = pos;

						bool found_resize_target_point = false;

						auto set_diffs_x = [&](const real32 size_diff_x, const real32 pos_diff_x) {
							desired_pos.x = (rect.l + rect.r + pos_diff_x) / 2;
							desired_size.x = current_size.x + size_diff_x;

							found_resize_target_point = true;
						};

						auto unitize_diff_x = [&](const real32 diff_x) {
							const auto would_be_w = static_cast<int>(current_size.x + diff_x);

							if (size_unit.has_value()) {
								auto snapped_w = would_be_w;

								snapped_w /= size_unit->x;
								snapped_w *= size_unit->x;

								snapped_w = std::max(snapped_w, size_unit->x);

								return snapped_w - current_size.x;
							}

							if (would_be_w < 1.f) {
								/* Be sensible */
								return 1.f - current_size.x;
							}

							return diff_x;
						};

						auto set_diffs_y = [&](const real32 size_diff_y, const real32 pos_diff_y) {
							desired_pos.y = (rect.t + rect.b + pos_diff_y) / 2;
							desired_size.y = current_size.y + size_diff_y;

							found_resize_target_point = true;
						};

						auto unitize_diff_y = [&](const real32 diff_y) {
							const auto would_be_h = static_cast<int>(current_size.y + diff_y);

							if (size_unit.has_value()) {
								auto snapped_h = would_be_h;

								snapped_h /= size_unit->y;
								snapped_h *= size_unit->y;

								snapped_h = std::max(snapped_h, size_unit->y);

								return snapped_h - current_size.y;
							}

							if (would_be_h < 1.f) {
								/* Be sensible */
								return 1.f - current_size.y;
							}

							return diff_y;
						};

						auto sref = ref;
						rect.snap_point(sref);

						if (const auto diff_x = ref.x - rect.r; edges.right && diff_x > 0) {
							const auto d = unitize_diff_x(diff_x);
							set_diffs_x(d, d);
						}
						else if (const auto diff_x = rect.l - ref.x; edges.left && diff_x > 0) {
							const auto d = unitize_diff_x(diff_x);
							set_diffs_x(d, -d);
						}
						else if (const auto diff_x = rect.r - sref.x; edges.right && diff_x > 0) {
							const auto d = unitize_diff_x(-diff_x);
							set_diffs_x(d, d);
						}
						else if (const auto diff_x = sref.x - rect.l; edges.left && diff_x > 0) {
							const auto d = unitize_diff_x(-diff_x);
							set_diffs_x(d, -d);
						}

						if (const auto diff_y = ref.y - rect.b; edges.bottom && diff_y > 0) {
							const auto d = unitize_diff_y(diff_y);
							set_diffs_y(d, d);
						}
						else if (const auto diff_y = rect.t - ref.y; edges.top && diff_y > 0) {
							const auto d = unitize_diff_y(diff_y);
							set_diffs_y(d, -d);
						}
						else if (const auto diff_y = rect.b - sref.y; edges.bottom && diff_y > 0) {
							const auto d = unitize_diff_y(-diff_y);
							set_diffs_y(d, d);
						}
						else if (const auto diff_y = sref.y - rect.t; edges.top && diff_y > 0) {
							const auto d = unitize_diff_y(-diff_y);
							set_diffs_y(d, -d);
						}

						if (found_resize_target_point) {
							set_size(desired_size);

							if (augs::is_nonzero(rot)) {
								desired_pos.rotate(original_transform);
							}

							set_pos(desired_pos);
						}
					},
					key
				);
			}
		}
	);

	(void)both_axes_simultaneously;
}

std::string move_nodes_command::describe() const {
	if (special == special_entity_mover_op::RESET_ROTATION) {
		return typesafe_sprintf("Reset rotation: %x", built_description);
	}

	if (rotation_center) {
		return typesafe_sprintf("Rotated by %x*: %x", static_cast<int>(move_by.rotation), built_description);
	}

	if (meta.is_child) {
		return typesafe_sprintf(built_description);
	}

	return typesafe_sprintf("Moved by %x: %x", move_by.pos, built_description);
}

void move_nodes_command::push_entry(const const_entity_handle handle) {
	handle.dispatch([&](const auto typed_handle) {
		using E = entity_type_of<decltype(typed_handle)>;
		using vector_type = typed_entity_id_vector<E>;

		moved_entities.get<vector_type>().push_back({ typed_handle.get_id() });
	});
}

void move_nodes_command::clear_entries() {
	moved_entities.clear();
}

void move_nodes_command::unmove_entities(const editor_command_input in) {
	auto& cosm = in.setup.get_cosmos();
	auto before_change_data = augs::cref_memory_stream(values_before_change);

	::unmove_entities({}, cosm, moved_entities, before_change_data);
}

void move_nodes_command::reinfer_moved(cosmos& cosm) {
	moved_entities.for_each([&](const auto id){
		const auto handle = cosm[id];	
		handle.infer_transform();
	});
}

void move_nodes_command::move_entities(cosmos& cosm) {
	::move_entities({}, cosm, moved_entities, move_by, rotation_center, special);
}

void move_nodes_command::rewrite_change(
	const delta_type& new_value,
	const editor_command_input in
) {
	auto& cosm = in.setup.get_cosmos();

	/* For improved determinism, client of this function should unmove the entities first... */
	/* ...and only now move by the new delta, exactly as if we were moving the entities for the first time. */
	move_by = new_value;
	move_entities(cosm);

	reinfer_moved(cosm);
	::read_back_to_nodes(in.setup);
}

void move_nodes_command::reselect_moved_entities(const editor_command_input in) {
	reselect(moved_entities, in.setup, in.setup.entity_selector_state);
}

void move_nodes_command::redo(const editor_command_input in) {
	clear_undo_state();

	auto& cosm = in.setup.get_cosmos();
	auto before_change_data = augs::ref_memory_stream(values_before_change);

	::save_transforms(cosm, moved_entities, before_change_data);
	move_entities(cosm);

	reselect_moved_entities(in);
	::read_back_to_nodes(in.setup);
}

void move_nodes_command::undo(const editor_command_input in) {
	unmove_entities(in);
	clear_undo_state();

	/*
		We are rebuilding the scene after every command anyway.
		No need to reinfer.
	*/

	//cosmic::reinfer_all_entities(cosm);

	reselect_moved_entities(in);
	::read_back_to_nodes(in.setup);
}

void move_nodes_command::clear_undo_state() {
	values_before_change.clear();
}

resize_nodes_command::active_edges::active_edges(const transformr tr, const vec2 rect_size, vec2 target_point, const bool both_axes) {
	target_point.rotate(-tr.rotation, tr.pos);

	const auto edges = ltrb::center_and_size(tr.pos, rect_size).make_edges(); 

	auto segment_closer = [&target_point](const auto& a, const auto& b) {
		return 
			target_point.sq_distance_from(a)
		   	< target_point.sq_distance_from(b)
		;
	};

	if (both_axes) {
		if (segment_closer(edges[0], edges[2])) {
			top = true;
			bottom = false;
		}
		else {
			top = false;
			bottom = true;
		}

		if (segment_closer(edges[1], edges[3])) {
			right = true;
			left = false;
		}
		else {
			right = false;
			left = true;
		}
	}
	else {
		const auto idx = std::addressof(minimum_of(edges, segment_closer)) - std::addressof(edges[0]);

		if (idx == 0) {
			top = true;
		}
		if (idx == 1) {
			right = true;
		}
		if (idx == 2) {
			bottom = true;
		}
		if (idx == 3) {
			left = true;
		}
	}
}

std::string resize_nodes_command::describe() const {
	return typesafe_sprintf("Resized %x", built_description);
}

void resize_nodes_command::push_entry(const const_entity_handle handle) {
	handle.dispatch([&](const auto typed_handle) {
		using E = entity_type_of<decltype(typed_handle)>;
		using vector_type = typed_entity_id_vector<E>;

		resized_entities.get<vector_type>().push_back({ typed_handle.get_id() });
	});
}

void resize_nodes_command::unresize_entities(cosmos& cosm) {
	auto before_change_data = augs::cref_memory_stream(values_before_change);

	::unresize_entities({}, cosm, resized_entities, before_change_data);
	::unmove_entities({}, cosm, resized_entities, before_change_data);
}

void resize_nodes_command::reinfer_resized(cosmos& cosm) {
	resized_entities.for_each([&](const auto id){
		const auto handle = cosm[id];	
		handle.infer_colliders_from_scratch();
		handle.infer_transform();
	});
}

void resize_nodes_command::resize_entities(cosmos& cosm) {
	if (!edges.is_set()) {
		resized_entities.for_each(
			[&](const auto& i) {
				const auto typed_handle = cosm[i];

				if (const auto tr = typed_handle.find_logic_transform()) {
					edges = active_resized_edges(*tr, typed_handle.get_logical_size(), target_point.actual, both_axes_simultaneously);
				}
			}
		);
	}

	::resize_entities({}, cosm, resized_entities, target_point, edges, both_axes_simultaneously);
}

void resize_nodes_command::rewrite_change(
	const point_type& new_resize_target_point,
	const editor_command_input in
) {
	auto& cosm = in.setup.get_cosmos();

	/* For improved determinism, unresize the entities first... */
	unresize_entities(cosm);

	/* ...and only now move by the new delta, exactly as if we were moving the entities for the first time. */
	target_point = new_resize_target_point;
	resize_entities(cosm);

	reinfer_resized(cosm);

	::read_back_to_nodes(in.setup);
}

void resize_nodes_command::redo(const editor_command_input in) {
	clear_undo_state();

	auto& cosm = in.setup.get_cosmos();
	auto before_change_data = augs::ref_memory_stream(values_before_change);

	::save_sizes(cosm, resized_entities, before_change_data);
	::save_transforms(cosm, resized_entities, before_change_data);

	resize_entities(cosm);

	//cosmic::reinfer_all_entities(cosm);

	reselect_resized_entities(in);
	::read_back_to_nodes(in.setup);
}

void resize_nodes_command::undo(const editor_command_input in) {
	auto& cosm = in.setup.get_cosmos();

	auto before_change_data = augs::cref_memory_stream(values_before_change);

	unresize_entities(cosm);
	clear_undo_state();

	//cosmic::reinfer_all_entities(cosm);

	reselect_resized_entities(in);
	::read_back_to_nodes(in.setup);
}

void resize_nodes_command::clear_undo_state() {
	values_before_change.clear();
}

void resize_nodes_command::reselect_resized_entities(const editor_command_input in) {
	reselect(resized_entities, in.setup, in.setup.entity_selector_state);
}

std::string flip_nodes_command::describe() const {
	if (flip.horizontally) {
		return "Flipped horizontally: " + built_description;
	}

	if (flip.vertically) {
		return "Flipped vertically: " + built_description;
	}

	return "Unknown flip operation: " + built_description;
}

void flip_nodes_command::push_entry(const const_entity_handle handle) {
	handle.dispatch([&](const auto typed_handle) {
		using E = entity_type_of<decltype(typed_handle)>;
		using vector_type = typed_entity_id_vector<E>;

		flipped_entities.get<vector_type>().push_back({ typed_handle.get_id() });
	});
}

void flip_nodes_command::clear_undo_state() {
	values_before_change.clear();
}

void flip_nodes_command::flip_entities(cosmos& cosm) {
	auto for_each_subject = [this](auto callback){ 
		flipped_entities.for_each([&](auto& e) {
			callback(e);
		});
	};

	if (const auto source_aabb = find_aabb_of(cosm, for_each_subject)) {
		auto flip_with_mirror = [&](auto calc_mirror_offset) {
			for_each_subject(
				[&](const auto& id) {
					const auto typed_handle = cosm[id];

					if (const auto ir = typed_handle.find_independent_transform()) {
						const auto source_transform = *ir;
						const auto new_rotation = source_transform.get_direction().neg_y().degrees();

						const auto mirror_offset = calc_mirror_offset(
							source_transform.pos, 
							typed_handle.find_aabb()
						);

						auto mirrored_transform = transformr(mirror_offset + source_transform.pos, new_rotation);
						fix_pixel_imperfections(mirrored_transform);

						/* 
							Areas and points get special treatment when calculating flipped rotations,
							because they do not support flip flags.
						*/

						const bool is_area = typed_handle.template find<invariants::area_marker>();

						if (is_area) {
							if (flip.vertically) {
								mirrored_transform.rotation += 180;
							}
						}
						else {
							/* It's a point */
							if (!typed_handle.do_flip(flip)) {
								if (flip.horizontally) {
									mirrored_transform.rotation += 180;
								}
							}
						}

						typed_handle.set_logic_transform(mirrored_transform);
					}
				}
			);
		};

		if (flip.horizontally) {
			flip_with_mirror(
				[source_aabb](const transformr& source, const std::optional<ltrb>& aabb) {
					if (aabb) {
						return vec2(source_aabb->l + source_aabb->r - aabb->l - aabb->r, 0.f);
					}
					else {
						const auto dist_to_axis = source_aabb->r - source.pos.x;
						return vec2(dist_to_axis * 2 - source_aabb->w(), 0.f);
					}
				}
			);
		}

		if (flip.vertically) {
			flip_with_mirror(
				[source_aabb](const transformr& source, const std::optional<ltrb>& aabb) {
					if (aabb) {
						return vec2(0.f, source_aabb->t + source_aabb->b - aabb->t - aabb->b);
					}
					else {
						const auto dist_to_axis = source_aabb->b - source.pos.y;
						return vec2(0.f, dist_to_axis * 2);
					}
				}
			);
		}
	}
}

void flip_nodes_command::redo(const editor_command_input in) {
	clear_undo_state();

	auto& cosm = in.setup.get_cosmos();
	auto before_change_data = augs::ref_memory_stream(values_before_change);

	::save_transforms(cosm, flipped_entities, before_change_data);
	flip_entities(cosm);

	//cosmic::reinfer_all_entities(cosm);

	reselect_flipped_entities(in);
	::read_back_to_nodes(in.setup);
}

void flip_nodes_command::unmove_entities(cosmos& cosm) {
	auto before_change_data = augs::cref_memory_stream(values_before_change);

	::unmove_entities({}, cosm, flipped_entities, before_change_data);
}

void flip_nodes_command::undo(const editor_command_input in) {
	auto& cosm = in.setup.get_cosmos();

	unmove_entities(cosm);

	flipped_entities.for_each([&](auto& id) {
		const auto typed_handle = cosm[id];

		if (typed_handle.find_independent_transform()) {
			typed_handle.do_flip(flip);
		}
	});

	clear_undo_state();

	//cosmic::reinfer_all_entities(cosm);

	reselect_flipped_entities(in);
	::read_back_to_nodes(in.setup);
}

void flip_nodes_command::reselect_flipped_entities(const editor_command_input in) {
	reselect(flipped_entities, in.setup, in.setup.entity_selector_state);
}
