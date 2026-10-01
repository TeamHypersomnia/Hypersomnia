#pragma once
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include "augs/graphics/vertex.h"
#include "game/cosmos/entity_id.h"
#include "game/stateless_systems/visibility_system.h"

/*
	What the light jobs of one frame did - summed up by the parallel jobs, for the profiler.
*/

struct light_jobs_stats {
	std::atomic<std::size_t> ray_casts = 0;
	std::atomic<std::size_t> aabb_queries = 0;
	std::atomic<std::size_t> lights_recalculated = 0;
	std::atomic<std::size_t> lights_reused = 0;
	std::atomic<std::size_t> cpu_microseconds = 0;

	void reset() {
		ray_casts = 0;
		aabb_queries = 0;
		lights_recalculated = 0;
		lights_reused = 0;
		cpu_microseconds = 0;
	}
};

/*
	The shadows a light's job computed the last time, reused for as long as nothing they depend on changes:
	the light itself, and every fixture within its reach, with its transform - see calc_light_shadows_fingerprint.
*/

struct light_shadows_cache_entry {
	std::size_t fingerprint = 0;
	bool valid = false;
	uint32_t last_used_frame = 0;

	visibility_response response;
	augs::vertex_triangle_buffer triangles;
	augs::vertex_triangle_buffer penumbras;
	augs::vertex_triangle_buffer shadow_masks;
	augs::vertex_triangle_buffer low_shadow_masks;
};

struct cached_visibility_data {
	visibility_response fow_response;
	std::vector<visibility_response> light_responses;
	std::vector<visibility_request> light_requests;

	std::unordered_map<unversioned_entity_id, light_shadows_cache_entry> light_shadows_cache;
	uint32_t current_frame = 0;

	light_jobs_stats light_stats;
};
