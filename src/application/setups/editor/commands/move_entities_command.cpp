#include "game/cosmos/cosmos_solvable_access.h"

#include "application/intercosm.h"
#include "application/setups/editor/editor_folder.h"
#include "application/setups/editor/editor_command_input.h"
#include "application/setups/editor/commands/move_entities_command.h"
#include "application/setups/editor/gui/editor_entity_selector.h"
#include "application/setups/editor/gui/find_aabb_of.h"
#include "application/setups/editor/detail/editor_transform_utils.h"

#include "augs/readwrite/memory_stream.h"
#include "augs/readwrite/byte_readwrite.h"

using delta_type = move_entities_command::delta_type;
using moved_entities_type = move_entities_command::moved_entities_type;
using resized_entities_type = resize_entities_command::resized_entities_type;

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

static void save_sizes(
	cosmos& cosm,
	const resized_entities_type& subjects,
	augs::ref_memory_stream& into
) {
	subjects.for_each(
		[&](const auto& i) {
			const auto typed_handle = cosm[i];

			if constexpr(typed_handle.template has<components::overridden_size>()) {
				const auto& overridden_size = typed_handle.get().template get<components::overridden_size>();

				augs::write_bytes(into, overridden_size.size);
			}
		}
	);
}

static void unresize_entities(
	const cosmos_solvable_access key,
	cosmos& cosm,
	const resized_entities_type& subjects,
	augs::cref_memory_stream& from
) {
	subjects.for_each(
		[&](const auto& i) {
			const auto typed_handle = cosm[i];

			if constexpr(typed_handle.template has<components::overridden_size>()) {
				auto& overridden_size = typed_handle.get(key).template get<components::overridden_size>();

				augs::read_bytes(from, overridden_size.size);
			}
		}
	);
}

static void move_entities(
	const cosmos_solvable_access key,
	cosmos& cosm,
	const moved_entities_type& subjects,
	const delta_type& dt,
	const std::optional<vec2> rotation_center
) {
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
					tr.set(tr.get() + dt_si);
				}
				else if constexpr(std::is_same_v<T, transformr>) {
					tr += dt;
				}
				else if constexpr(std::is_same_v<T, vec2>) {
					tr += dt.pos;
				}
				else {
					static_assert(always_false_v<T>, "Unknown transform type.");
				}
			}
		;

		on_each_independent_transform(cosm, subjects, mover, key);
	}
}

static void resize_entities(
	const cosmos_solvable_access key,
	cosmos& cosm,
	const resized_entities_type& subjects,
	const vec2& world_ref,
	const active_edges& edges,
	const bool both_axes_simultaneously
) {
	const auto si = cosm.get_si();

	subjects.for_each(
		[&](const auto& i) {
			const auto typed_handle = cosm[i];
			std::optional<vec2i> size_unit;

			if constexpr(typed_handle.template has<invariants::sprite>()) {
				const auto& spr = typed_handle.template get<invariants::sprite>();

				if (spr.tile_excess_size) { 
					size_unit = spr.size;
				}
			}

			if constexpr(typed_handle.template has<components::overridden_size>()) {
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

						const auto ref = vec2(world_ref).rotate(-rot, pos);
						const auto rect = ltrb::center_and_size(pos, current_size);

						auto set_size = [&](const vec2 new_size) {
							auto& overridden_size = typed_handle.get(key).template get<components::overridden_size>();

							if (size_unit.has_value()) {
								vec2i s = new_size;
								s /= size_unit.value();
								s *= size_unit.value();
								overridden_size.size.emplace(s);
							}
							else {
								overridden_size.size.emplace(new_size);
							}
						};

						auto set_pos = [&](const vec2 new_pos) {
							if constexpr(std::is_same_v<T, physics_engine_transforms>) {
								auto t = transform.get();
								t.pos = si.get_meters(new_pos);
								transform.set(t);
							}
							else if constexpr(std::is_same_v<T, transformr>) {
								transform.pos = new_pos;
							}
							else if constexpr(std::is_same_v<T, vec2>) {
								transform = new_pos;
							}
							else {
								static_assert(always_false_v<T>);
							}
						};

						vec2 desired_size = current_size;
						vec2 desired_pos = pos;

						bool found_reference_point = false;

						auto set_diffs_x = [&](const real32 size_diff_x, const real32 pos_diff_x) {
							desired_pos.x = (rect.l + rect.r + pos_diff_x) / 2;
							desired_size.x = current_size.x + size_diff_x;

							found_reference_point = true;
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

							found_reference_point = true;
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

						if (found_reference_point) {
							set_size(desired_size);

							if (!augs::is_epsilon(rot)) {
								desired_pos.mult(original_transform);
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

std::string move_entities_command::describe() const {
	if (rotation_center) {
		return typesafe_sprintf("Rotated by %x*: %x", static_cast<int>(move_by.rotation), built_description);
	}

	return typesafe_sprintf("Moved by %x: %x", move_by.pos, built_description);
}

void move_entities_command::push_entry(const const_entity_handle handle) {
	handle.dispatch([&](const auto typed_handle) {
		using E = entity_type_of<decltype(typed_handle)>;
		using vector_type = typed_entity_id_vector<E>;

		moved_entities.get<vector_type>().push_back({ typed_handle.get_id() });
	});
}

void move_entities_command::unmove_entities(cosmos& cosm) {
	auto before_change_data = augs::cref_memory_stream(values_before_change);

	::unmove_entities({}, cosm, moved_entities, before_change_data);
}

void move_entities_command::reinfer_moved(cosmos& cosm) {
	moved_entities.for_each([&](const auto id){
		const auto handle = cosm[id];	
		handle.infer_transform();
	});
}

void move_entities_command::move_entities(cosmos& cosm) {
	::move_entities({}, cosm, moved_entities, move_by, rotation_center);
}

void move_entities_command::rewrite_change(
	const delta_type& new_value,
	const editor_command_input in
) {
	auto& cosm = in.get_cosmos();

	/* For improved determinism, client of this function should unmove the entities first... */
	/* ...and only now move by the new delta, exactly as if we were moving the entities for the first time. */
	move_by = new_value;
	move_entities(cosm);

	reinfer_moved(cosm);
}

void move_entities_command::redo(const editor_command_input in) {
	auto& cosm = in.get_cosmos();

	auto before_change_data = augs::ref_memory_stream(values_before_change);
	ensure(values_before_change.empty());

	save_transforms(cosm, moved_entities, before_change_data);
	move_entities(cosm);

	cosmic::reinfer_all_entities(cosm);

	auto& selections = in.folder.view.selected_entities;
	selections.clear();

	moved_entities.for_each([&](const auto id) {
		selections.emplace(id);
	});
}

void move_entities_command::undo(const editor_command_input in) {
	auto& cosm = in.get_cosmos();

	unmove_entities(cosm);
	values_before_change.clear();

	cosmic::reinfer_all_entities(cosm);

	in.folder.view.select_ids(moved_entities);
}


active_edges::active_edges(const transformr tr, const vec2 rect_size, vec2 reference_point, const bool both_axes) {
	reference_point.rotate(-tr.rotation, tr.pos);

	const auto edges = ltrb::center_and_size(tr.pos, rect_size).make_edges(); 

	auto segment_closer = [&reference_point](const auto& a, const auto& b) {
		return 
			reference_point.distance_from_segment_sq(a)
		   	< reference_point.distance_from_segment_sq(b)
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
		*(std::addressof(top) + idx) = true;
	}
}

std::string resize_entities_command::describe() const {
	return typesafe_sprintf("Resized %x", built_description);
}

void resize_entities_command::push_entry(const const_entity_handle handle) {
	handle.dispatch([&](const auto typed_handle) {
		using E = entity_type_of<decltype(typed_handle)>;
		using vector_type = typed_entity_id_vector<E>;

		resized_entities.get<vector_type>().push_back({ typed_handle.get_id() });
	});
}

void resize_entities_command::unresize_entities(cosmos& cosm) {
	auto before_change_data = augs::cref_memory_stream(values_before_change);

	::unresize_entities({}, cosm, resized_entities, before_change_data);
	::unmove_entities({}, cosm, resized_entities, before_change_data);
}

void resize_entities_command::reinfer_resized(cosmos& cosm) {
	resized_entities.for_each([&](const auto id){
		const auto handle = cosm[id];	
		handle.infer_colliders_from_scratch();
		handle.infer_transform();
	});
}

void resize_entities_command::resize_entities(cosmos& cosm) {
	if (edges == active_edges()) {
		resized_entities.for_each(
			[&](const auto& i) {
				const auto typed_handle = cosm[i];

				if (const auto tr = typed_handle.find_logic_transform()) {
					edges = active_edges(*tr, typed_handle.get_logical_size(), reference_point.actual, both_axes_simultaneously);
				}
			}
		);
	}

	::resize_entities({}, cosm, resized_entities, reference_point.snapped, edges, both_axes_simultaneously);
}

void resize_entities_command::rewrite_change(
	const point_type& new_reference_point,
	const editor_command_input in
) {
	auto& cosm = in.get_cosmos();

	/* For improved determinism, unresize the entities first... */
	unresize_entities(cosm);

	/* ...and only now move by the new delta, exactly as if we were moving the entities for the first time. */
	reference_point = new_reference_point;
	resize_entities(cosm);

	reinfer_resized(cosm);
}

void resize_entities_command::redo(const editor_command_input in) {
	auto& cosm = in.get_cosmos();

	auto before_change_data = augs::ref_memory_stream(values_before_change);
	ensure(values_before_change.empty());

	::save_sizes(cosm, resized_entities, before_change_data);
	::save_transforms(cosm, resized_entities, before_change_data);

	resize_entities(cosm);

	cosmic::reinfer_all_entities(cosm);

	in.folder.view.select_ids(resized_entities);
}

void resize_entities_command::undo(const editor_command_input in) {
	auto& cosm = in.get_cosmos();

	auto before_change_data = augs::cref_memory_stream(values_before_change);

	unresize_entities(cosm);
	values_before_change.clear();

	cosmic::reinfer_all_entities(cosm);

	in.folder.view.select_ids(resized_entities);
}
