#pragma once
#include <cstddef>
#include "view/rendering_scripts/vis_response_to_triangles.h"
#include "game/enums/filters.h"

inline void enqueue_visibility_jobs(
	augs::thread_pool& pool,

	const cosmos& cosm,
	augs::dedicated_buffers& dedicated,
	cached_visibility_data& cached_visibility,

	const bool fow_effective,
	const entity_id subject,
	const transformr viewed_character_transform,
	const fog_of_war_settings& fog_of_war
) {
	using DV = augs::dedicated_buffer_vector;
	using D = augs::dedicated_buffer;

	auto launch_light_jobs = [&]() {
		const auto& light_requests = cached_visibility.light_requests;
		const auto lights_n = light_requests.size();

		auto& light_responses = cached_visibility.light_responses;
		light_responses.resize(lights_n);

		auto& light_triangles_vectors = dedicated[DV::LIGHT_VISIBILITY];
		light_triangles_vectors.resize(lights_n);

		for (std::size_t i = 0; i < lights_n; ++i) {
			const auto& request = light_requests[i];
			auto& response = light_responses[i];

			if (!request.valid()) {
				response.clear();
				continue;
			}

			auto& triangles = light_triangles_vectors[i].triangles;

			auto light_job = [&cosm, request, &response, &triangles]() {
				visibility_system(DEBUG_FRAME_LINES).calc_visibility(cosm, request, response);
				vis_response_to_triangles(response, triangles, request.color, request.eye_transform.pos);
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
			visibility_system(DEBUG_FRAME_LINES).calc_visibility(cosm, request, fow_response);
			vis_response_to_triangles(fow_response, fow_triangles, white, request.eye_transform.pos);
		};

		pool.enqueue(fow_job);
	}
}

