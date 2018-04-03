#pragma once
#include <optional>

template <class T>
std::optional<ltrb> find_editor_aabb_of(const T handle) {
	if (handle.template find<components::light>() && handle.template find<invariants::light>()) {
		/* Light-like */
		return xywh::center_and_size(handle.get_logic_transform().pos, vec2(2, 2));	
	}

	return handle.find_aabb();
};

template <class C>
void combine_aabb_of(ltrb& total, C& cosm, const entity_id id) {
	const auto handle = cosm[id];

	if (const auto aabb = find_editor_aabb_of(handle)) {
		total.contain(*aabb);
	}
};

template <class C, class F>
std::optional<ltrb> find_aabb_of(
	const C& cosm,
	F for_each_target
) {
	ltrb total;

	auto combine = [&total, &cosm](const entity_id id) {
		::combine_aabb_of(total, cosm, id);
	};

	for_each_target(combine);

	if (total.good()) {
		return total;
	}

	return std::nullopt;
}
