#pragma once
#include <vector>
#include <utility>
#include <algorithm>
#include "augs/math/polygon_queries.h"
#include "game/cosmos/entity_id.h"
#include "game/enums/render_layer.h"
#include "game/components/decal_component.h"
#include "game/components/sentience_component.h"
#include "game/detail/decals/character_gunshot_drip.h"
#include "game/detail/physics/world_polygon.h"

/*
	The visible bullet holes and drips (render_layer::CHARACTER_DECALS), grouped by the body they are attached to -
	gathered once a frame, so that each corpse draws its own. The drips of a body first, so that they are drawn
	under its holes; then by id, so that overlapping ones don't swap places from frame to frame.

	Only corpses draw them - a living character's arms move with every animation frame and the holes would float
	where they were, see character_gunshot_decal.h. A lying corpse draws all of its own, as they were fitted into it
	when it fell; a standing one only those wholly within its colliders as they are now - one hit while holding
	a rifle may be out in the air once the rifle has fallen from its hands.
*/

struct owned_decals {
	const cosmos* cosm = nullptr;
	struct entry {
		entity_id owner;
		bool is_hole = false;
		entity_id decal;
	};

	std::vector<entry> by_owner;

	template <class V>
	void gather(const cosmos& in_cosm, const V& visible) {
		cosm = std::addressof(in_cosm);
		by_owner.clear();

		const auto& assets = in_cosm.get_common_assets();

		visible.template for_each<render_layer::CHARACTER_DECALS>(in_cosm, [&](const auto& handle) {
			handle.template dispatch_on_having_all<components::decal>([&](const auto& typed_decal) {
				by_owner.push_back({
					typed_decal.template get<components::decal>().attached_to,
					!::is_character_gunshot_drip(assets, typed_decal.get_raw_flavour_id()),
					typed_decal.get_id()
				});
			});
		});

		std::sort(by_owner.begin(), by_owner.end(), [](const auto& a, const auto& b) {
			if (a.owner != b.owner) {
				return a.owner < b.owner;
			}

			if (a.is_hole != b.is_hole) {
				return !a.is_hole;
			}

			return a.decal < b.decal;
		});
	}

	auto find_first_of(const entity_id owner) const {
		return std::lower_bound(by_owner.begin(), by_owner.end(), owner, [](const auto& e, const entity_id id) {
			return e.owner < id;
		});
	}

	bool has_any_of(const entity_id owner) const {
		const auto it = find_first_of(owner);
		return it != by_owner.end() && it->owner == owner;
	}

	template <class F>
	void for_each_of(const entity_id owner, F&& callback) const {
		for (auto it = find_first_of(owner); it != by_owner.end() && it->owner == owner; ++it) {
			(*cosm)[it->decal].template dispatch_on_having_all<components::decal>(callback);
		}
	}

	template <class H, class F>
	void for_each_within_standing_corpse(const H& body, F&& callback) const {
		const auto* const sentience = body.template find<components::sentience>();

		if (sentience == nullptr || !sentience->is_dead() || sentience->has_exploded || !has_any_of(body.get_id())) {
			return;
		}

		const auto body_polygons = ::get_body_polygons_px(body, cosm->get_si());

		for_each_of(body.get_id(), [&](const auto& typed_decal) {
			const auto pos = typed_decal.get_logic_transform().pos;
			const auto radius = typed_decal.get_logical_size().length() / 2;

			if (augs::is_circle_within_polygons(body_polygons, pos, radius)) {
				callback(typed_decal);
			}
		});
	}
};

inline owned_decals& thread_local_owned_decals() {
	thread_local owned_decals decals;
	return decals;
}
