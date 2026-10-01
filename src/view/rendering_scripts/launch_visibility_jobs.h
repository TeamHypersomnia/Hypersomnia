#pragma once
#include "augs/math/transform.h"
#include "game/cosmos/entity_id.h"

class cosmos;
struct cached_visibility_data;
struct fog_of_war_settings;

namespace augs {
	class thread_pool;
	struct dedicated_buffers;
}

/*
	Enqueues the visibility of every light and of the fog of war as jobs.
	Lights reuse the shadows computed at the earlier frames while nothing they depend on changes -
	see light_shadows_cache_entry.
*/

void enqueue_visibility_jobs(
	augs::thread_pool& pool,

	const cosmos& cosm,
	augs::dedicated_buffers& dedicated,
	cached_visibility_data& cached_visibility,

	bool fow_effective,
	entity_id subject,
	transformr viewed_character_transform,
	const fog_of_war_settings& fog_of_war,
	real32 point_light_shadow_smoothness_multiplier,
	bool point_light_soft_shadows,
	bool point_light_heights
);
