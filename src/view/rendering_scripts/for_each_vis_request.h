#pragma once

class particles_simulation_system;

template <class F, class C>
void for_each_light_vis_request(
	F callback,

	const cosmos& cosm,
	const visible_entities& visible,
	const C& per_entity_cache,
	const interpolation_system& interp,
	const ltrb queried_camera_aabb,
	const particles_simulation_system* const particles_system
) {
	/* 
		TODO_PERFORMANCE: If we're going to use visible here, we must fix problems with lights' aabb detection,
		and probably somehow ascertain that transforms are reinferred properly, e.g. there were some problems in testbed.
	*/

	(void)visible;
	cosm.template for_each_having<components::light>(
		[&](const auto light_entity) {
			const auto light_transform = light_entity.get_viewing_transform(interp);
			const auto& light = light_entity.template get<components::light>();

			const auto reach = light.calc_reach_trimmed();
			const auto light_aabb = xywh::center_and_size(light_transform.pos, reach);

			if (const auto cache = mapped_or_nullptr(per_entity_cache, unversioned_entity_id(light_entity))) {
				const auto light_displacement = vec2(cache->all_variation_values[6], cache->all_variation_values[7]);

				messages::visibility_information_request request;

				request.eye_transform = light_transform;
				request.eye_transform.pos += light_displacement;

				if (queried_camera_aabb.hover(light_aabb)) {
					request.queried_rect = reach;
				}
				else {
					request.queried_rect = {};
				}

				request.filter = predefined_queries::light();
				request.subject = light_entity;
				request.color = light.color;

				/*
					Store light data and cache in the request.
				*/
				request.light_data = light;
				request.variation_cache = cache->all_variation_values;

				callback(request);
			}
		}
	);

	/*
		Add visibility requests for temporary lights (muzzle flashes, etc.)
	*/

	if (particles_system != nullptr) {
		for (const auto& temp_light : particles_system->temporary_lights) {
			/*
				Temporary lights don't have entity associations or cache entries,
				so we construct the light data on-the-fly.
			*/

			const auto light_component = temp_light.to_light_component();
			const auto reach = light_component.calc_reach_trimmed();
			const auto light_aabb = xywh::center_and_size(temp_light.pos, reach);

			messages::visibility_information_request request;

			request.eye_transform.pos = temp_light.pos;
			request.eye_transform.rotation = 0.0f;

			if (queried_camera_aabb.hover(light_aabb)) {
				request.queried_rect = reach;
			}
			else {
				request.queried_rect = {};
			}

			if (temp_light.cast_shadow) {
				request.filter = predefined_queries::muzzle_light();
			}
			else {
				request.filter = predefined_queries::light();
			}

			request.subject = entity_id();
			request.color = temp_light.color;

			/*
				Store the constructed light data.
				No variation cache for temporary lights.
			*/
			request.light_data = light_component;
			request.variation_cache = std::nullopt;

			callback(request);
		}
	}
}

