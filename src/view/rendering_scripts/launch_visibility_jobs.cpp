#include <algorithm>
#include <chrono>
#include <cstddef>
#include "augs/ensure.h"
#include "augs/misc/scope_guard.h"
#include "augs/templates/container_templates.h"
#include "augs/templates/hash_templates.h"
#include "augs/templates/thread_pool.h"
#include "augs/graphics/vertex.h"
#include "augs/graphics/dedicated_buffers.h"
#include "game/enums/filters.h"
#include "game/cosmos/cosmos.h"
#include "game/stateless_systems/visibility_system.h"
#include "game/modes/detail/fog_of_war_settings.h"
#include "view/rendering_scripts/vis_response_to_triangles.h"
#include "view/rendering_scripts/light_height_shadows.h"
#include "view/rendering_scripts/shadow_casters.h"
#include "view/rendering_scripts/launch_visibility_jobs.h"
#include "application/main/cached_visibility_data.h"

/*
	Cache entries of lights out of sight for this many frames are dropped.
*/
constexpr uint32_t light_shadows_cache_keep_frames_v = 120;

/*
	Everything the shadows of a light depend on: its own parameters - the color too, baked into the vertices -
	and every fixture within its reach,
	with its transform and shadow height - summed per fixture, so that the order of the query doesn't matter.
	The reach is widened a little, for the neighbors that decide whether a silhouette's side gets a penumbra.
*/

static std::size_t calc_light_shadows_fingerprint(
	const cosmos& cosm,
	const visibility_request& request,
	const real32 smoothness
) {
	const auto& light = request.light_data;
	const auto pos = request.eye_transform.pos;

	auto fixtures_sum = std::size_t(0);
	auto num_fixtures = std::size_t(0);

	const auto half_reach = vec2(request.queried_rect) / 2 + vec2::square(4.0f);

	cosm.get_solvable_inferred().physics.for_each_in_aabb(
		cosm.get_si(),
		pos - half_reach,
		pos + half_reach,
		request.filter,
		[&](const b2Fixture& fix) {
			const auto& xf = fix.GetBody()->GetTransform();

			fixtures_sum += augs::hash_multiple(
				reinterpret_cast<std::uintptr_t>(std::addressof(fix)),
				xf.p.x,
				xf.p.y,
				xf.q.s,
				xf.q.c,
				::calc_fixture_shadow_height(cosm, fix)
			);

			++num_fixtures;

			return callback_result::CONTINUE;
		}
	);

	return augs::hash_multiple(
		pos.x,
		pos.y,
		request.queried_rect.x,
		request.queried_rect.y,
		request.filter.categoryBits,
		request.filter.maskBits,
		light.height,
		smoothness,
		light.attenuation.calc_reach(),
		request.subject.raw.indirection_index,
		request.color.r,
		request.color.g,
		request.color.b,
		request.color.a,
		fixtures_sum,
		num_fixtures
	);
}

void enqueue_visibility_jobs(
	augs::thread_pool& pool,

	const cosmos& cosm,
	augs::dedicated_buffers& dedicated,
	cached_visibility_data& cached_visibility,

	const bool fow_effective,
	const entity_id subject,
	const transformr viewed_character_transform,
	const fog_of_war_settings& fog_of_war,
	const real32 point_light_shadow_smoothness_multiplier,
	const bool point_light_soft_shadows,
	const bool point_light_heights
) {
	using DV = augs::dedicated_buffer_vector;
	using D = augs::dedicated_buffer;

	auto launch_light_jobs = [&]() {
		/*
			The settings switching features off apply to the requests,
			so that the light system draws the lights consistently with how their jobs computed them.
		*/

		for (auto& light_request : cached_visibility.light_requests) {
			if (!point_light_soft_shadows) {
				light_request.light_data.shadow_smoothness = 0.0f;
			}

			if (!point_light_heights) {
				light_request.light_data.height = 0.0f;
			}
		}

		const auto& light_requests = cached_visibility.light_requests;
		const auto lights_n = light_requests.size();

		auto& light_responses = cached_visibility.light_responses;
		light_responses.resize(lights_n);

		auto& light_triangles_vectors = dedicated[DV::LIGHT_VISIBILITY];
		light_triangles_vectors.resize(lights_n);

		auto& light_penumbras_vectors = dedicated[DV::LIGHT_PENUMBRAS];
		light_penumbras_vectors.resize(lights_n);

		auto& light_shadow_masks_vectors = dedicated[DV::LIGHT_SHADOW_MASKS];
		light_shadow_masks_vectors.resize(lights_n);

		auto& light_low_shadow_masks_vectors = dedicated[DV::LIGHT_LOW_SHADOW_MASKS];
		light_low_shadow_masks_vectors.resize(lights_n);

		const auto smoothness_mult = 
			cosm.get_common_significant().light.point_light_shadows.smoothness_mult 
			* point_light_shadow_smoothness_multiplier
		;

		auto& stats = cached_visibility.light_stats;
		stats.reset();

		/*
			Entries are found or made here, before the jobs start - each job then touches only its own.
		*/

		auto& shadows_cache = cached_visibility.light_shadows_cache;
		const auto frame = ++cached_visibility.current_frame;

		erase_if(shadows_cache, [frame](const auto& it) {
			return frame - it.second.last_used_frame > light_shadows_cache_keep_frames_v;
		});

		for (std::size_t i = 0; i < lights_n; ++i) {
			const auto& request = light_requests[i];
			auto& response = light_responses[i];

			auto& penumbras = light_penumbras_vectors[i].triangles;
			auto& shadow_masks = light_shadow_masks_vectors[i].triangles;
			auto& low_shadow_masks = light_low_shadow_masks_vectors[i].triangles;

			if (!request.valid()) {
				response.clear();
				penumbras.clear();
				shadow_masks.clear();
				low_shadow_masks.clear();
				continue;
			}

			auto& triangles = light_triangles_vectors[i].triangles;

			/*
				Temporary lights, like muzzle flashes, have no entity to keep their shadows for.
			*/

			auto* const cache_entry = [&]() -> light_shadows_cache_entry* {
				if (!request.subject.is_set()) {
					return nullptr;
				}

				auto& entry = shadows_cache[unversioned_entity_id(request.subject)];
				entry.last_used_frame = frame;

				return std::addressof(entry);
			}();

			auto light_job = [&cosm, request, &response, &triangles, &penumbras, &shadow_masks, &low_shadow_masks, smoothness_mult, &stats, cache_entry]() {
				const auto counters_before = thread_physics_query_counters;
				const auto started = std::chrono::steady_clock::now();

				bool reused = false;

				auto add_stats = augs::scope_guard([&]() {
					const auto& counters = thread_physics_query_counters;
					const auto elapsed = std::chrono::steady_clock::now() - started;

					stats.ray_casts += counters.ray_casts - counters_before.ray_casts;
					stats.aabb_queries += counters.aabb_queries - counters_before.aabb_queries;
					(reused ? stats.lights_reused : stats.lights_recalculated) += 1;
					stats.cpu_microseconds += static_cast<std::size_t>(std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count());
				});

				const auto smoothness = std::clamp(request.light_data.shadow_smoothness * smoothness_mult, 0.0f, 1.0f);

				const auto fingerprint = cache_entry != nullptr ? ::calc_light_shadows_fingerprint(cosm, request, smoothness) : std::size_t(0);

				if (cache_entry != nullptr && cache_entry->valid && cache_entry->fingerprint == fingerprint) {
					response = cache_entry->response;
					triangles = cache_entry->triangles;
					penumbras = cache_entry->penumbras;
					shadow_masks = cache_entry->shadow_masks;
					low_shadow_masks = cache_entry->low_shadow_masks;

					reused = true;
					return;
				}

				auto store_in_cache = augs::scope_guard([&]() {
					if (cache_entry != nullptr) {
						cache_entry->fingerprint = fingerprint;
						cache_entry->valid = true;
						cache_entry->response = response;
						cache_entry->triangles = triangles;
						cache_entry->penumbras = penumbras;
						cache_entry->shadow_masks = shadow_masks;
						cache_entry->low_shadow_masks = low_shadow_masks;
					}
				});

				if (request.light_data.height > 0.0f) {
					/*
						Lights with a height don't use the visibility polygon.
					*/

					response.clear();
					triangles.clear();
					penumbras.clear();

					::build_light_shadow_masks(
						{
							cosm,
							request.eye_transform.pos,
							request.queried_rect,
							request.filter,
							request.subject,
							request.light_data.height,
							smoothness,
							MAX_LIGHT_PENUMBRA_DEGREES,
							request.color,
							request.light_data.attenuation.calc_reach()
						},
						shadow_masks,
						low_shadow_masks
					);

					return;
				}

				shadow_masks.clear();
				low_shadow_masks.clear();

				visibility_system().calc_visibility(cosm, request, response);
				vis_response_to_triangles(response, triangles, request.color, request.eye_transform.pos);

				append_light_penumbras(
					response,
					penumbras,
					request.color,
					request.eye_transform.pos,
					smoothness,
					cosm.get_solvable_inferred().physics,
					cosm.get_si(),
					request.filter
				);
			};

			pool.enqueue(light_job);
		}
	};

	launch_light_jobs();

	if (fow_effective) {
		const auto fow_size = fog_of_war.get_real_size();

		visibility_request request;
		request.eye_transform = viewed_character_transform;
		request.filter = predefined_queries::line_of_sight();
		request.queried_rect = fow_size;
		request.subject = subject;

		auto& fow_response = cached_visibility.fow_response;
		auto& fow_triangles = dedicated[D::FOG_OF_WAR].triangles;

		auto fow_job = [request, &cosm, &fow_response, &fow_triangles]() {
			visibility_system().calc_visibility(cosm, request, fow_response);
			vis_response_to_triangles(fow_response, fow_triangles, white, request.eye_transform.pos);
		};

		pool.enqueue(fow_job);
	}
}

