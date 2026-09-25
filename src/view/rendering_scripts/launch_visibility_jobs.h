#pragma once
#include <algorithm>
#include <cstddef>
#include "view/rendering_scripts/vis_response_to_triangles.h"
#include "view/rendering_scripts/light_height_shadows.h"
#include "game/enums/filters.h"

inline void enqueue_visibility_jobs(
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

		const auto smoothness_mult = 
			cosm.get_common_significant().light.point_light_shadow_smoothness_mult 
			* point_light_shadow_smoothness_multiplier
		;

		for (std::size_t i = 0; i < lights_n; ++i) {
			const auto& request = light_requests[i];
			auto& response = light_responses[i];

			auto& penumbras = light_penumbras_vectors[i].triangles;
			auto& shadow_masks = light_shadow_masks_vectors[i].triangles;

			if (!request.valid()) {
				response.clear();
				penumbras.clear();
				shadow_masks.clear();
				continue;
			}

			auto& triangles = light_triangles_vectors[i].triangles;

			auto light_job = [&cosm, request, &response, &triangles, &penumbras, &shadow_masks, smoothness_mult]() {
				const auto smoothness = std::clamp(request.light_data.shadow_smoothness * smoothness_mult, 0.0f, 1.0f);

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
							MAX_LIGHT_PENUMBRA_DEGREES
						},
						shadow_masks
					);

					return;
				}

				shadow_masks.clear();

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

