#include "application/setups/editor/editor_camera.h"
#include "application/setups/editor/editor_view.h"
#include "application/setups/editor/editor_settings.h"
#include "augs/templates/reversion_wrapper.h"

namespace editor_detail {
	bool handle_camera_input(
		const editor_camera_settings& settings,
		camera_cone cone,
		const augs::event::state& common_input_state,
		const augs::event::change e,
		const vec2 world_cursor_pos,
		camera_eye& panned_camera
	) {
		using namespace augs::event;
		using namespace augs::event::keys;

		const bool has_shift{ common_input_state[key::LSHIFT] };
		const bool has_alt{ common_input_state[key::LALT] };
		const bool has_ctrl{ common_input_state[key::LCTRL] };

		const auto pan_mult = [&](){
			float result = 1.f;

			if (has_shift) {
				result *= 5;
			}

			if (has_alt) {
				result /= 5;
			}

			return result;
		}();

		const auto zoom_mult = [&](){
			float result = 1.f;

			if (has_shift) {
				result *= 5;
			}

			if (has_alt) {
				result /= 5;
			}

			return result;
		}();

		auto pan_scene = [&](const auto amount) {
			auto& camera = panned_camera;

			camera.transform.pos -= pan_mult * amount / camera.zoom;
		};

		auto set_zoom_with_pos_adjustment = [&panned_camera](const auto new_zoom, const auto zoom_point) {
			auto& camera = panned_camera;
			const auto old_zoom = camera.zoom;

			camera.zoom = new_zoom;	
			camera.transform.pos += (1 - 1 / (new_zoom/old_zoom))*(zoom_point - camera.transform.pos);
			camera.transform.pos.discard_fract();
		};

		auto zoom_scene = [set_zoom_with_pos_adjustment, &panned_camera, &cone, zoom_mult](const auto zoom_amount, const auto zoom_point) {
			auto& camera = panned_camera;
			(void)cone;

			const auto min_zoom = 0.01f;
			const auto max_zoom = 10.f;

			const auto zoom_offset = 0.09f * camera.zoom * zoom_amount * zoom_mult;
			const auto new_zoom = std::clamp(camera.zoom + zoom_offset, min_zoom, max_zoom);

			set_zoom_with_pos_adjustment(new_zoom, zoom_point);
		};

		if (e.msg == message::wheel) {
			zoom_scene(e.data.scroll.amount, world_cursor_pos);
			return true;
		}

#if 0
		if (e.was_released(key::RMOUSE)) {
			if (panned_camera.has_value()) {
				panned_camera->transform.pos = vec2i(panned_camera->transform.pos);
			}
		}
#endif

		if (e.msg == message::mousemotion && common_input_state[key::RMOUSE]) {
			pan_scene(vec2(e.data.mouse.rel) * settings.panning_speed);
			return true;
		}

		if (e.was_any_key_pressed()) {
			const auto k = e.data.key.key;

			const auto key_pan_amount = 50.f;

			const float zoom_levels[] = {
				0.05f,
				0.1f,
				0.25f,
				0.5f,
				0.75f,
				1.f,
				1.5f,
				2.0f,
				3.0f,
				4.0f,
				8.0f,
				16.0f
			};

			auto get_next_zoom_level = [&](const float current) {
				for (const auto zoom : zoom_levels) {
					if (zoom > current) {
						return zoom;
					}
				}

				return 16.0f;
			};

			auto get_prev_zoom_level = [&](const float current) {
				for (const auto zoom : reverse(zoom_levels)) {
					if (zoom < current) {
						return zoom;
					}
				}

				return 0.05f;
			};

			switch (k) {
				case key::UP: pan_scene(vec2(0, key_pan_amount)); return true;
				case key::DOWN: pan_scene(vec2(0, -key_pan_amount)); return true;
				case key::RIGHT: pan_scene(vec2(-key_pan_amount, 0)); return true;
				case key::LEFT: pan_scene(vec2(key_pan_amount, 0)); return true;

				case key::MINUS: 
				case key::SUBTRACT: 
					panned_camera.zoom = get_prev_zoom_level(panned_camera.zoom); 
					return true;

				case key::Z: 
					if (!has_shift && !has_ctrl) {
						panned_camera.zoom = 1.0f;
						return true;
					}

				case key::PLUS: 
				case key::ADD: 
						panned_camera.zoom = get_next_zoom_level(panned_camera.zoom);
						return true;

				case key::EQUAL: 
					if (has_shift) {
						panned_camera.zoom = get_next_zoom_level(panned_camera.zoom);
					}
					else {
						panned_camera.zoom = 1.0f;
					}

					return true;

				case key::HOME: panned_camera = {}; return true;
				default: break;
			}
		}

		return false;
	}
}
