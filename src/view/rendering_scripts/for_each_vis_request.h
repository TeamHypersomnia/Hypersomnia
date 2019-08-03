#pragma once

template <class F, class C>
void for_each_vis_request(
	F callback,

	const cosmos& cosm,
	const C& per_entity_cache,
	const interpolation_system& interp,
	const ltrb queried_camera_aabb
) {
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
					request.filter = predefined_queries::line_of_sight();
					request.queried_rect = reach;
					request.subject = light_entity;
					request.color = light.color;

					callback(request);
				}
			}
		}
	);
}

