#include "application/intercosm.h"

#include "application/setups/debugger/debugger_camera.h"
#include "application/setups/debugger/debugger_view.h"
#include "application/setups/debugger/debugger_player.h"
#include "application/setups/debugger/debugger_settings.h"

#include "game/cosmos/entity_handle.h"
#include "game/detail/inventory/inventory_slot_handle.h"

namespace debugger_detail {
	std::optional<camera_eye> calculate_camera(
		const debugger_player& player,
		const debugger_view& view,
		const const_entity_handle matching_go_to_entity,
		const const_entity_handle viewed_character
	) {
		std::optional<transformr> viewed_transform;

		const auto panning = view.panned_camera;

		if (viewed_character) {
			viewed_transform = viewed_character.find_logic_transform();
		}

		if (player.is_editing_mode()) {
			if (const auto match = matching_go_to_entity) {
				camera_eye centered_on_match;

				if (panning) {
					/* Propagate zoom taken from custom panning */
					centered_on_match = *panning;
				}

				if (const auto transform = match.find_logic_transform()) {
					centered_on_match.transform.pos = transform->pos;
				}

				return centered_on_match;
			}
			else if (panning) {
				return panning;
			}

			if (viewed_transform) {
				camera_eye centered_on_viewed;
				centered_on_viewed.transform.pos = viewed_transform->pos;

				return centered_on_viewed;
			}

			return camera_eye();
		}

		if (panning) {
			return panning;
		}

		if (!viewed_character) {
			return camera_eye();
		}

		return std::nullopt;
	}

	bool handle_camera_input(
		const debugger_camera_settings& settings,
		camera_cone cone,
		const augs::event::state& common_input_state,
		const augs::event::change e,
		const vec2 world_cursor_pos,
		std::optional<camera_eye>& panned_camera
	) {
		using namespace augs::event;
		using namespace augs::event::keys;

		const auto screen_size = cone.screen_size;
		const auto world_screen_center = cone.to_world_space(screen_size/2);

		const bool has_shift{ common_input_state[key::LSHIFT] };
		const bool has_alt{ common_input_state[key::LALT] };

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
			if (!panned_camera.has_value()) {
				panned_camera = cone.eye;
			}

			auto& camera = *panned_camera;

			camera.transform.pos -= pan_mult * amount / camera.zoom;
		};

		auto zoom_scene = [&panned_camera, &cone, zoom_mult](const auto zoom_amount, const auto zoom_point) {
			if (!panned_camera.has_value()) {
				panned_camera = cone.eye;
			}

			auto& camera = *panned_camera;

			const auto min_zoom = 0.01f;
			const auto max_zoom = 10.f;

			const auto old_zoom = camera.zoom;
			const auto zoom_offset = 0.09f * old_zoom * zoom_amount * zoom_mult;
			const auto new_zoom = std::clamp(old_zoom + zoom_offset, min_zoom, max_zoom);

			camera.zoom = new_zoom;	
			camera.transform.pos += (1 - 1 / (new_zoom/old_zoom))*(zoom_point - camera.transform.pos);
			camera.transform.pos.discard_fract();
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
			const auto key_zoom_amount = 1.f;

			switch (k) {
				case key::UP: pan_scene(vec2(0, key_pan_amount)); return true;
				case key::DOWN: pan_scene(vec2(0, -key_pan_amount)); return true;
				case key::RIGHT: pan_scene(vec2(-key_pan_amount, 0)); return true;
				case key::LEFT: pan_scene(vec2(key_pan_amount, 0)); return true;

				case key::MINUS: zoom_scene(-key_zoom_amount, world_screen_center); return true;
				case key::EQUAL: zoom_scene(key_zoom_amount, world_screen_center); return true;

				case key::HOME: panned_camera = std::nullopt; return true;
				default: break;
			}
		}

		return false;
	}
}
