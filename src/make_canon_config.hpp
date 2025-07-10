#pragma once

inline void make_canon_config(config_json_table& result, bool is_dedicated_server) {
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
			result.server_start.port = 8412;
		}

#if !IS_PRODUCTION_BUILD
		result.server_start.port = 8412;
#endif

#if PLATFORM_WEB
		using key_type = augs::event::keys::key;

		/* Point to HTTPS one */
		result.server_list_provider = "https://masterserver.hypersomnia.xyz:8420";

		result.window.fullscreen = false;
		result.window.border = false;
		result.sound.max_short_sounds = 32;

		result.game_controls.erase(key_type::LCTRL);

		/* Some rebinding is necessary for the Web */
		result.game_controls[key_type::C] = game_intent_type::TOGGLE_WALK_SILENTLY;
		result.game_controls[key_type::V] = game_intent_type::WIELD_BOMB;

		result.game_controls[key_type::K] = game_intent_type::THROW_PED_GRENADE;
		result.game_controls[key_type::L] = game_intent_type::THROW_FLASHBANG;

		result.game_controls.erase(key_type::MOUSE4);
		result.game_controls.erase(key_type::MOUSE5);

		result.general_gui_controls[key_type::T] = general_gui_intent_type::BUY_MENU;
		result.general_gui_controls.erase(key_type::B);

		result.general_gui_controls.erase(key_type::TILDE);
		result.general_gui_controls[key_type::HOME] = general_gui_intent_type::TOGGLE_MOUSE_CURSOR;

		result.inventory_gui_controls[key_type::B] = inventory_gui_intent_type::SPECIAL_ACTION_BUTTON_3;
		result.inventory_gui_controls.erase(key_type::V);

		result.sound.max_simultaneous_bullet_trace_sounds = 0;
		result.content_regeneration.rescan_assets_on_window_focus = false;

		result.client.nickname = "Guest";
#endif

#if WEB_LOWEND
		const auto canon_sz  = result.gui_fonts.gui.size_in_pixels;
		const auto target_sz = 18.0f;

		result.gui_fonts *= target_sz / canon_sz;
		result.gui_fonts.gui.size_in_pixels = target_sz;

		/*
			Should be fine as we're limiting the rate of audio commands
			to avoid hiccups.
		*/

		result.audio.output_mode = audio_output_mode::STEREO_HRTF;

		result.sound.max_short_sounds = 16;
		result.sound.processing_frequency = sound_processing_frequency::PERIODIC;
		result.sound.custom_processing_frequency = 2;
#endif

#if WEB_CRAZYGAMES
		auto& app_controls = result.app_controls;

		if (const auto k = key_or_default(app_controls, app_intent_type::SHOW_PERFORMANCE); k != key_type()) {
			app_controls.erase(k);
		}

		if (const auto k = key_or_default(app_controls, app_intent_type::SHOW_LOGS); k != key_type()) {
			app_controls.erase(k);
		}
#endif

#if !WEB_SINGLETHREAD
		/* The subsequent ones are for the "Loading..." screen which is only needed in singlethreaded build */
		result.gui_fonts.medium_numbers.unicode_ranges.resize(1);
#endif
}
