#include "augs/log.h"
#include "augs/drawing/drawing.hpp"
#include "augs/misc/action_list/standard_actions.h"

#include "augs/filesystem/file.h"
#include "augs/string/string_templates.h"

#include "augs/audio/sound_data.h"
#include "augs/audio/sound_buffer.h"
#include "augs/audio/sound_source.h"

#include "augs/gui/text/caret.h"
#include "augs/gui/text/printer.h"
#include "augs/drawing/drawing.h"

#include "game/assets/all_logical_assets.h"

#include "game/organization/all_component_includes.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/logic_step.h"
#include "game/organization/all_messages_includes.h"
#include "game/cosmos/data_living_one_step.h"

#include "game/detail/visible_entities.h"

#include "application/config_json_table.h"
#include "augs/readwrite/json_readwrite.h"

#include "application/setups/main_menu_setup.h"

#include "application/gui/menu/menu_root.h"
#include "application/gui/menu/menu_context.h"
#include "application/gui/menu/appearing_text.h"
#include "application/gui/menu/creators_screen.h"

#include "hypersomnia_version.h"
#include "application/main/self_updater.h"

#include "application/arena/choose_arena.h"
#include "application/setups/editor/packaged_official_content.h"

#include "augs/misc/web_sdk_events.h"

using namespace augs::event::keys;
using namespace augs::gui::text;
using namespace augs::gui;

#define MENU_LOGO_IN_GUI 1

entity_id main_menu_setup::get_viewed_character_id() const {
	return viewed_character_id;
}

struct main_menu_setup_detail {
	rapidjson::Document patch;
};

void main_menu_setup::customize_for_viewing(config_json_table& config) const {
	const auto previous_sfx_volume = config.audio_volume.sound_effects;
	augs::read_json(detail->patch, config);
	
	/* Treat new volume as a multiplier */

	config.audio_volume.sound_effects *= previous_sfx_volume;
	config.drawing.cinematic_mode = true;
	config.drawing.custom_zoom = 3.0f;
}

void main_menu_setup::apply(const config_json_table& config) {
	menu_theme_source.set_gain(config.audio_volume.music);
}

void main_menu_setup::launch_creators_screen() {
#if TODO
	if (credits_actions.is_complete()) {
		creators = creators_screen();
		creators.setup(textes_style, style(assets::font_id::GUI_FONT, { 0, 180, 255, 255 }), window.get_screen_size());

		credits_actions.push_blocking(act(new augs::tween_value_action<rgba_channel>(fade_overlay_color.a, 170, 500.f)));
		creators.push_into(credits_actions);
		credits_actions.push_blocking(act(new augs::tween_value_action<rgba_channel>(fade_overlay_color.a, 20, 500.f)));
	}
#endif
}

main_menu_setup::main_menu_setup(
	const packaged_official_content& official,
	const main_menu_settings settings
) : detail(std::make_unique<main_menu_setup_detail>()) {
	auto loading_raii = web_sdk_loading_raii();

	LOG("main_menu_setup ctor");

	if (!settings.menu_theme_path.empty()) {
		try {
			menu_theme.emplace(settings.menu_theme_path);
		}
		catch (...) {
			// LOG("Warning: could not load the main menu theme:\n%x", err.what());
		}
	}

	const auto menu_config_patch_path = augs::path_type("content/menu/config.json");

	try {
		LOG("Reading from %x", menu_config_patch_path);

		detail->patch = augs::json_document_from(menu_config_patch_path);
	}
	catch (const augs::json_deserialization_error& err) {
		LOG("Failed to load %x:\n%x\nMenu will apply no patch to config.", menu_config_patch_path, err.what());
	}
	catch (const augs::file_open_error& err) {
		LOG("Failed to load %x:\n%x\nMenu will apply no patch to config.", menu_config_patch_path, err.what());
	}

	if (menu_theme) {
		menu_theme_source.bind_buffer(*menu_theme);
		menu_theme_source.set_direct_channels(true);
		menu_theme_source.set_spatialize(false);
		menu_theme_source.seek_to(static_cast<float>(settings.start_menu_music_at_secs));
		menu_theme_source.set_gain(0.f);
		menu_theme_source.play();
	}

	// TODO: actually load a cosmos with its resources from a file/folder
	const bool is_intro_scene_available = settings.menu_background_arena_path.string().size() > 0;

	auto& cosm = scene.world;

	if (is_intro_scene_available) {
		editor_project project;

		all_modes_variant mv;
		all_rulesets_variant rv;

		cosmos_solvable_significant dummy;

		current_arena_folder = settings.menu_background_arena_path;
		auto paths = editor_project_paths(current_arena_folder);

		auto handle = online_arena_handle<false>{ 
			mv,
			scene,
			scene.world,
			rv,
			dummy,
			dummy_dynamic_vars
		};

		::load_arena_from_path(
			{
				editor_project_readwrite::reading_settings(),
				handle,
				official,
				"",
				"",
				dummy,
				std::nullopt,
				&project,
				nullptr
			},

			paths.project_json,
			nullptr
		);

		if (auto m = std::get_if<test_mode>(&mv)) {
			mode = *m;
		}

		if (auto r = std::get_if<test_mode_ruleset>(&rv)) {
			ruleset = *r;
		}

		viewed_character_id = cosm[mode.lookup(mode.add_player({ dummy_dynamic_vars, ruleset, cosm }, "Player", faction_type::METROPOLIS))].get_id();
	}

	const bool is_recording_available = is_intro_scene_available && false;
	initial_step_number = cosm.get_total_steps_passed();

#if 0
	gui.root.buttons[main_menu_button_type::QUICK_PLAY].set_appearing_caption("Connect to official server");
	gui.root.buttons[main_menu_button_type::BROWSE_SERVERS].set_appearing_caption("Browse servers");
	gui.root.buttons[main_menu_button_type::HOST_SERVER].set_appearing_caption("Host server");
	gui.root.buttons[main_menu_button_type::CONNECT_TO_SERVER].set_appearing_caption("Connect to server");
	gui.root.buttons[main_menu_button_type::SHOOTING_RANGE].set_appearing_caption("Shooting range");
	gui.root.buttons[main_menu_button_type::EDITOR].set_appearing_caption("Editor");
	gui.root.buttons[main_menu_button_type::SETTINGS].set_appearing_caption("Settings");
	gui.root.buttons[main_menu_button_type::CREATORS].set_appearing_caption("Founders");
	gui.root.buttons[main_menu_button_type::QUIT].set_appearing_caption("Quit");
#endif

	if (is_recording_available) {
		while (cosm.get_total_seconds_passed() < settings.rewind_intro_scene_by_secs) {
			mode.advance(
				{ dummy_dynamic_vars, ruleset, cosm },
				mode_entropy(),
				solver_callbacks(),
				solve_settings()
			);
		}
	}
}

main_menu_setup::~main_menu_setup() = default;

void main_menu_setup::draw_overlays(
	const self_update_result& last_update_result,
	const augs::drawer_with_default output,
	const necessary_images_in_atlas_map& necessarys,
	const augs::baked_font& gui_font,
	const vec2i screen_size
) const {
#if MENU_LOGO_IN_GUI
	const auto game_logo = necessarys.at(assets::necessary_image_id::MENU_GAME_LOGO);
	const auto game_logo_size = game_logo.get_original_size();

	ltrb game_logo_rect;
	game_logo_rect.set_position({ 70.0f, 50.f });
	game_logo_rect.set_size(game_logo_size);

	const bool draw_menu_logo = true;

	if (draw_menu_logo) {
		output.aabb(game_logo, game_logo_rect);
	}
#endif

	(void)necessarys;

	const bool show_latest_news = false;

	if (show_latest_news) {
		print_stroked(
			output,
			vec2i(latest_news_pos),
			from_bbcode ( typesafe_sprintf("[color=green]Latest change:[/color] %x", hypersomnia_version().commit_message), { gui_font, white } )
		);
	}

	const auto s = style { gui_font, white };

	using FS = formatted_string;
	const auto build_number_text = FS(typesafe_sprintf("Build %x", hypersomnia_version().get_version_string()), s);

	auto colored = [&](const auto& text, const auto& color) {
		return FS(std::string(text) + " ", { gui_font, color });
	};

	const auto description = [&]() {
#if PLATFORM_WEB
		return colored("", gray);
#endif
		const auto t = last_update_result.type;
		using R = self_update_result_type;

		switch (t) {
			case R::NONE:
				return colored("Automatic updates are disabled.", gray);
			case R::CANCELLED:
				return colored("Automatic update was cancelled!", orange);
			case R::FIRST_LAUNCH_AFTER_UPGRADE:
				return colored("Success! Hypersomnia was upgraded to the latest version.", green);
			case R::COULDNT_DOWNLOAD_VERSION_FILE:
				return colored("Version file not found on the update server!", red);
			case R::COULDNT_DOWNLOAD_BINARY:
				return colored("Latest binary not found on the update server!", red);
			case R::FAILED_TO_VERIFY_BINARY:
				return colored("FAILED TO AUTHENTICATE THE UPDATE! The official server might be compromised!!!", red);
			case R::COULDNT_SAVE_BINARY:
				return colored("Failed to save the downloaded binary! Ensure you have sufficient space.", red);
			case R::FAILED_TO_OPEN_SSH_KEYGEN:
				return colored("Failed to open ssh-keygen! The updated version's signature could not be verified.", red);
			case R::DOWNLOADED_BINARY_WAS_OLDER:
				return colored("The update server has provided an older version of the game.", red);
			case R::UP_TO_DATE:
				return colored("Hypersomnia is up to date.", green);
			case R::FAILED:
				return colored("Failed to connect with the update server.", red);
			case R::UPDATE_AVAILABLE:
				return colored("Updates are available. Restart to install.", yellow);

			default: return colored("", white);
		}
	}();
	
	const auto bbox = get_text_bbox(build_number_text);
	const auto dbbox = get_text_bbox(description);
	(void)bbox;
	(void)dbbox;

	vec2i padding = { 6, 6 };

	print_stroked(
		output,
#if WEB_LOWEND
		vec2i(screen_size.x - padding.x, padding.y),
#else
		vec2i(screen_size.x, screen_size.y) - padding,
#endif
		description + build_number_text,
#if WEB_LOWEND
		{ augs::ralign::R, augs::ralign::T }
#else
		{ augs::ralign::R, augs::ralign::B }
#endif
	);
}

void main_menu_setup::get_steam_rich_presence_pairs(steam_rich_presence_pairs& pairs) const {
	pairs.push_back({ "steam_display", "#Status_AtMainMenu" });
}