#pragma once
#include "view/mode_gui/arena/arena_gui_mixin.h"
#include "application/setups/draw_setup_gui_input.h"
#include "view/client_arena_type.h"

class client_setup;

template <class D>
std::optional<camera_eye> arena_gui_mixin<D>::find_current_camera_eye() const {
	const auto& self = static_cast<const D&>(*this);

	if (!self.is_gameplay_on()) {
		return camera_eye();
	}

	return std::nullopt;
#if 0
	return self.get_arena_handle().on_mode(
		[&](const auto& typed_mode) -> std::optional<camera_eye> {
			if (const auto player = typed_mode.find(self.get_local_player_id())) {
				if (player->get_faction() == faction_type::SPECTATOR) {
					return camera_eye();
				}
			}

			return std::nullopt;
		}
	);
#endif
}

template <class D>
setup_escape_result arena_gui_mixin<D>::escape() {
	auto& self = static_cast<D&>(*this);

	if (!self.is_gameplay_on()) {
		return setup_escape_result::IGNORE;
	}

	if (arena_gui.escape()) {
		return setup_escape_result::JUST_FETCH;
	}

	return setup_escape_result::IGNORE;
}

template <class D>
custom_imgui_result arena_gui_mixin<D>::perform_custom_imgui(
	const perform_custom_imgui_input in
) {
	auto& self = static_cast<D&>(*this);

	if (!self.is_gameplay_on()) {
		return custom_imgui_result::NONE;
	}

	const auto game_screen_top = 0.f;

	const auto draw_mode_in = draw_mode_gui_input { 
		game_screen_top, 
		self.get_local_player_id(), 
		in.game_atlas,
		in.config,
		in.demo_replay_mode
	};

	auto perform_with = [&](const prediction_input input, const auto... args) {
		self.get_arena_handle(args...).on_mode_with_input(
			[&](const auto& typed_mode, const auto& mode_input) {
				const auto new_entropy = arena_gui.perform_imgui_and_advance(
					draw_mode_in, 
					typed_mode, 
					mode_input,
					input
				);

				self.control(new_entropy);
			}
		);
	};

	if constexpr(std::is_same_v<client_setup, D>) {
		if (self.is_spectating_referential()) {
			perform_with(prediction_input::offline());
		}
		else {
			perform_with(prediction_input::unpredictable_for(self.get_viewed_character()), client_arena_type::REFERENTIAL);
			perform_with(prediction_input::predictable_for(self.get_viewed_character()), client_arena_type::PREDICTED);
		}
	}
	else {
		perform_with(prediction_input::offline());
	}

	return custom_imgui_result::NONE;
}

template <class D>
bool arena_gui_mixin<D>::handle_input_before_imgui(
	const handle_input_before_imgui_input in
) {
	const auto& self = static_cast<const D&>(*this);

	if (!self.is_gameplay_on()) {
		return false;
	}

	(void)in;
	return false;
}

template <class D>
bool arena_gui_mixin<D>::handle_input_before_game(
	const handle_input_before_game_input in
) {
	const auto& self = static_cast<const D&>(*this);

	if (!self.is_gameplay_on()) {
		return false;
	}

	if (arena_gui.control({ in.app_controls, in.game_controls, in.common_input_state, in.e })) { 
		return true;
	}

	return false;
}

template <class D>
void arena_gui_mixin<D>::draw_custom_gui(const draw_setup_gui_input& in) const {
	const auto& self = static_cast<const D&>(*this);

	if (!self.is_gameplay_on()) {
		return;
	}

	const auto game_screen_top = 0.f;

	const auto draw_mode_in = draw_mode_gui_input { 
		game_screen_top,
		self.get_local_player_id(), 
		in.images_in_atlas,
		in.config,
		in.demo_replay_mode
	};

	auto draw_with = [&](const prediction_input input, const auto... args) {
		self.get_arena_handle(args...).on_mode_with_input([&](const auto& typed_mode, const auto& mode_input) {
			arena_gui.draw_mode_gui(in, draw_mode_in, typed_mode, mode_input, input);
		});
	};

	if constexpr(std::is_same_v<client_setup, D>) {
		if (self.is_spectating_referential()) {
			draw_with(prediction_input::offline());
		}
		else {
			draw_with(prediction_input::unpredictable_for(self.get_viewed_character()), client_arena_type::REFERENTIAL);
			draw_with(prediction_input::predictable_for(self.get_viewed_character()), client_arena_type::PREDICTED);
		}
	}
	else {
		draw_with(prediction_input::offline());
	}
}


template <class D>
std::string arena_gui_mixin<D>::get_scoreboard_caption() const {
	const auto& self = static_cast<const D&>(*this);

	return typesafe_sprintf(
		"%x - %x",
		self.get_current_arena_name(),
		self.get_arena_handle().get_current_game_mode_name()
	);
}

template <class D>
bool arena_gui_mixin<D>::requires_cursor() const {
	if (arena_gui.requires_cursor()) {
		return true;
	}

	if constexpr (std::is_same_v<client_setup, D>) {
		const auto& self = static_cast<const D&>(*this);

		if (self.client_gui.rcon.show) {
			return true;
		}
	}

	return false;
}

template <class D>
entity_id arena_gui_mixin<D>::get_game_gui_subject_id() const {
	const auto& self = static_cast<const D&>(*this);

	if (!self.is_gameplay_on()) {
		return self.get_viewed_character_id();
	}

	auto get_viewed_or_local_with = [&](const auto... args) {
		return self.get_arena_handle(args...).on_mode_with_input([&](const auto& typed_mode, const auto& mode_input) {
			const bool show_details_of_enemies = !mode_input.rules.should_hide_details_when_spectating_enemies();

			if (show_details_of_enemies) {
				return self.get_viewed_character_id();
			}

			if (!arena_gui.spectator.active) {
				return self.get_viewed_character_id();
			}

			const auto local_player_id = self.get_local_player_id();
			const auto local_player_data = typed_mode.find(local_player_id);

			const auto viewed_player_id = arena_gui.spectator.now_spectating;
			const auto viewed_player_data = typed_mode.find(viewed_player_id);

			if (viewed_player_data == nullptr || local_player_data == nullptr) {
				return self.get_viewed_character_id();
			}

			if (local_player_data->get_faction() == faction_type::SPECTATOR) {
				return self.get_viewed_character_id();
			}

			if (viewed_player_data->get_faction() != local_player_data->get_faction()) {
				return local_player_data->controlled_character_id;
			}

			return self.get_viewed_character_id();
		});
	};

	if constexpr(std::is_same_v<client_setup, D>) {
		if (self.is_replaying()) {
			return self.get_viewed_character_id();
		}

		return get_viewed_or_local_with(client_arena_type::PREDICTED);
	}
	else {
		return get_viewed_or_local_with();
	}
}

template <class D>
entity_id arena_gui_mixin<D>::get_viewed_character_id() const {
	const auto& self = static_cast<const D&>(*this);

	if (!self.is_gameplay_on()) {
		return entity_id::dead();
	}

	return self.get_arena_handle().on_mode_with_input(
		[&](const auto& typed_mode, const auto& in) {
			(void)in;

			if (arena_gui.spectator.active) {
				const auto spectating = arena_gui.spectator.now_spectating;

				if (spectating.is_set()) {
					return typed_mode.lookup(spectating);
				}
			}

			const auto local_id = self.get_local_player_id();
			const auto local_character = typed_mode.lookup(local_id);

			return local_character;
		}
	);
}

template <class D>
vec2 arena_gui_mixin<D>::get_viewed_player_nonzoomedout_visible_world_area() const {
	const auto& self = static_cast<const D&>(*this);

	if (!self.is_gameplay_on()) {
		return vec2::zero;
	}

	const auto mode_id = [&]() {
		if (arena_gui.spectator.active) {
			const auto spectating = arena_gui.spectator.now_spectating;

			if (spectating.is_set()) {
				return spectating;
			}
		}

		return self.get_local_player_id();
	}();

	if (mode_id.is_set()) {
		if (auto metas = self.find_player_metas()) {
			return (*metas)[mode_id.value].synced.public_settings.nonzoomedout_visible_world_area;
		}
	}

	return vec2::zero;
}
