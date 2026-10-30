#pragma once
#include <vector>
#include <utility>
#include <algorithm>
#include "game/cosmos/entity_id.h"
#include "game/enums/render_layer.h"
#include "game/components/decal_component.h"
#include "game/detail/decals/character_gunshot_drip.h"

/*
	The visible bullet holes of lying corpses (render_layer::CHARACTER_DECALS),
	grouped by the body they are attached to - gathered once a frame, so that each corpse draws its own.
	The drips of a body first, so that they are drawn under its holes.
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

			return !a.is_hole && b.is_hole;
		});
	}

	template <class F>
	void for_each_of(const entity_id owner, F&& callback) const {
		auto it = std::lower_bound(by_owner.begin(), by_owner.end(), owner, [](const auto& e, const entity_id id) {
			return e.owner < id;
		});

		for (; it != by_owner.end() && it->owner == owner; ++it) {
			callback(it->decal);
		}
	}
};

inline owned_decals& thread_local_owned_decals() {
	thread_local owned_decals decals;
	return decals;
}
