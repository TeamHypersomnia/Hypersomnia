#pragma once

inline void make_canon_config(config_lua_table& result, bool is_dedicated_server) {
#if !IS_PRODUCTION_BUILD
		/* Some developer-friendly options */
		result.self_update.update_on_launch = false;
		result.client.suppress_webhooks = true;
		result.server.suppress_new_community_server_webhook = true;
		result.unit_tests.run = true;
		result.audio_volume.music = 0;

#if PLATFORM_UNIX
		result.window.fullscreen = false;
#endif
#endif

		/* Tweak performance defaults */

		const auto concurrency = std::thread::hardware_concurrency();

		bool hrtf = true;

		if (concurrency <= 4) {
			hrtf = false;
			result.window.max_fps.value = 60;
		}

#if !NDEBUG
		hrtf = false;
#endif

		if (hrtf) {
			result.audio.output_mode = audio_output_mode::STEREO_HRTF;
		}
		else {
			result.audio.output_mode = audio_output_mode::AUTO;
		}

#if PLATFORM_LINUX
		/* 
			We don't yet properly implement detecting whether the app is in fs or not. 
			So better use the system cursor so that we don't clip it accidentally. 
		*/

		result.window.draw_own_cursor_in_fullscreen = false;
#else
		result.window.draw_own_cursor_in_fullscreen = true;
#endif

#if PLATFORM_MACOS
		result.window.fullscreen = true;
#endif

#if PLATFORM_WINDOWS
		result.drawing.stencil_before_light_pass = true;
#else
		result.drawing.stencil_before_light_pass = false;
#endif

#if HEADLESS
		result.server.allow_nat_traversal = false;
#endif

		if (is_dedicated_server) {
			result.server.allow_nat_traversal = false;
		}

#if PLATFORM_WEB
		/* Point to HTTPS one */
		result.server_list_provider = "https://masterserver.hypersomnia.xyz:8420";

		result.window.fullscreen = false;
		result.window.border = false;
		result.sound.max_short_sounds = 32;

		result.game_controls.erase(augs::event::keys::key::LCTRL);

		/* Some rebinding is necessary for the Web */
		result.game_controls[augs::event::keys::key::C] = game_intent_type::TOGGLE_WALK_SILENTLY;
		result.game_controls[augs::event::keys::key::V] = game_intent_type::WIELD_BOMB;

		result.game_controls[augs::event::keys::key::K] = game_intent_type::THROW_PED_GRENADE;
		result.game_controls[augs::event::keys::key::L] = game_intent_type::THROW_FLASHBANG;

		result.game_controls.erase(augs::event::keys::key::MOUSE4);
		result.game_controls.erase(augs::event::keys::key::MOUSE5);

		result.general_gui_controls[augs::event::keys::key::T] = general_gui_intent_type::BUY_MENU;
		result.general_gui_controls.erase(augs::event::keys::key::B);

		result.general_gui_controls.erase(augs::event::keys::key::TILDE);
		result.general_gui_controls[augs::event::keys::key::HOME] = general_gui_intent_type::TOGGLE_MOUSE_CURSOR;

		result.inventory_gui_controls[augs::event::keys::key::B] = inventory_gui_intent_type::SPECIAL_ACTION_BUTTON_3;
		result.inventory_gui_controls.erase(augs::event::keys::key::V);
#endif
}
