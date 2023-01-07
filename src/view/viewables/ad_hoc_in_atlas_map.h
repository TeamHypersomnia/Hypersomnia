#pragma once
#include <unordered_map>
#include "view/viewables/image_in_atlas.h"
#include "game/assets/animation_math.h"

using ad_hoc_entry_id = uint32_t;

struct ad_hoc_in_atlas_map {
	struct frame {
		augs::atlas_entry entry;
		float duration_milliseconds = 0.0f;
	};

	struct animation {
		std::vector<frame> frames;
	};

	std::unordered_map<ad_hoc_entry_id, animation> entries;
	mutable float animation_time = 0.0f;

	void add_frame(ad_hoc_entry_id id, augs::atlas_entry entry, float ms) {
		entries[id].frames.push_back({ entry, ms });
	}

	const augs::atlas_entry* find(const ad_hoc_entry_id& id) const {
		if (const auto anim = mapped_or_nullptr(entries, id)) {
			return std::addressof(calc_current_frame_looped(*anim, animation_time * 1000).entry);
		}

		return nullptr;
	}
};
