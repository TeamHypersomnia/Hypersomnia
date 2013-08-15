#include "camera_system.h"
#include "entity_system/entity.h"
#include "../components/physics_component.h"

#include <gl\GL.h>

camera_system::camera_system(render_system& raw_renderer) : raw_renderer(raw_renderer) {}

void camera_system::add(entity* e) {
	//e->get<components::camera>().interpolated_previous = e->get<components::transform>().current.pos;
	processing_system::add(e);
}

void camera_system::process_entities(world&) {
	/* we sort layers in reverse order to keep layer 0 as topmost and last layer on the bottom */
	std::sort(targets.begin(), targets.end(), [](entity* a, entity* b) {
		return a->get<components::camera>().layer > b->get<components::camera>().layer;
	});

	for (auto e : targets) {
		auto& camera = e->get<components::camera>();

		if(camera.enabled) {
			glLoadIdentity();
			glOrtho(camera.ortho.l, camera.ortho.r, camera.ortho.b, camera.ortho.t, 0, 1);
			glViewport(camera.screen_rect.x, camera.screen_rect.y, camera.screen_rect.w, camera.screen_rect.h);

			auto& transform = e->get<components::transform>();
			components::transform result_transform = transform;

			vec2<> camera_screen = vec2<>(vec2<int>(camera.ortho.w(), camera.ortho.h()));

			if (camera.player && camera.crosshair) {
				auto physics = camera.player->get<components::physics>();
				

				if (camera.orbit_mode != camera.NONE) {
					vec2<>& crosshair_pos = camera.crosshair->get<components::transform>().current.pos;
					vec2<>  player_pos = camera.player->get<components::transform>().current.pos;
					vec2<> dir = (crosshair_pos - player_pos);

					if (camera.orbit_mode == camera.ANGLED) {
						vec2<> bound = camera_screen / 2.f + camera.angled_look_length;
						vec2<> normalized = dir.clamp(bound);
						normalized.normalize();
						result_transform.current.pos += normalized * camera.angled_look_length;
					}

					if (camera.orbit_mode == camera.LOOK) {
						vec2<> bound = camera.max_look_expand + camera_screen / 2.f;

						result_transform.current.pos += (dir.clamp(bound) / bound) * camera.max_look_expand;
					}

					crosshair_pos = player_pos + dir;
				}
			}

			if (camera.enable_smoothing) {
				//if (transform.previous.pos != result_transform.current.pos) {
				//	camera.interpolated_previous = camera.last_interpolant;
				//	camera.animator.reset(0.f, 1.f
				//		//	, (result_transform.current.pos - transform.previous.pos).length() / camera_screen.length() * camera.miliseconds_transition
				//		);
				//}

				//float ratio = 0.f;
				//camera.animator.animate(ratio);
				//camera.last_interpolant = camera.interpolated_previous.lerp(result_transform.current.pos, ratio);


				float constant = static_cast<float>(pow(camera.smoothing_average_factor, camera.averages_per_sec*camera.smooth_timer.extract<std::chrono::seconds>()));
				camera.last_interpolant = camera.last_interpolant * constant + result_transform.current.pos*(1.f - constant);

				raw_renderer.draw(camera.ortho, components::transform(camera.last_interpolant), camera.mask);
				//transform.previous.pos = result_transform.current.pos;
			}
			else {
				raw_renderer.draw(camera.ortho, components::transform(result_transform.current.pos), camera.mask);
			}

		}
	}

	raw_renderer.render();
}