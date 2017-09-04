#include "augs/misc/lua_readwrite.h"
#include "augs/misc/http_requests.h"
#include "augs/misc/standard_actions.h"

#include "augs/filesystem/file.h"
#include "augs/templates/string_templates.h"

#include "augs/audio/sound_data.h"
#include "augs/audio/sound_buffer.h"
#include "augs/audio/sound_source.h"

#include "augs/gui/text/caret.h"
#include "augs/drawing/drawing.h"

#include "game/assets/all_assets.h"

#include "game/organization/all_component_includes.h"
#include "game/view/network/step_packaged_for_network.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/cosmic_movie_director.h"
#include "game/organization/all_messages_includes.h"
#include "game/transcendental/data_living_one_step.h"

#include "game/detail/visible_entities.h"

#include "test_scenes/test_scenes_content.h"
#include "test_scenes/scenes/testbed.h"
#include "test_scenes/scenes/minimal_scene.h"

#include "application/config_lua_table.h"

#include "application/setups/main_menu_setup.h"

#include "application/gui/menu/menu_root.h"
#include "application/gui/menu/menu_context.h"
#include "application/gui/menu/appearing_text.h"
#include "application/gui/menu/creators_screen.h"

#include "generated/introspectors.h"

using namespace augs::event::keys;
using namespace augs::gui::text;
using namespace augs::gui;

void main_menu_setup::customize_for_viewing(config_lua_table& config) {
	const auto previous_sfx_volume = config.audio_volume.sound_effects;
	augs::read(menu_config_patch, config);
	
	/* Treat new volume as a multiplier */

	config.audio_volume.sound_effects *= previous_sfx_volume;
}

void main_menu_setup::apply(const config_lua_table& config) {
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

main_menu_setup::main_menu_setup(const main_menu_settings settings) : menu_theme(settings.menu_theme_path) {
	auto& lua = augs::get_thread_local_lua_state();
	
	const auto menu_config_patch_path = "content/menu/config.lua";

	try {
		auto pfr = lua.do_string(augs::get_file_contents(menu_config_patch_path));
		
		if (pfr.valid()) {
			menu_config_patch = pfr;
		}
		else {
			LOG("Warning: problem loading %x: \n%x.", menu_config_patch_path, pfr.operator std::string());
		}
	}
	catch (const augs::ifstream_error& err) {
		LOG("Error loading file %x:\n%x\nMenu will apply no patch to config.", menu_config_patch_path, err.what());
	}

	float gain_fade_multiplier = 0.f;

	if (augs::file_exists(settings.menu_theme_path)) {
		menu_theme_source.bind_buffer(menu_theme);
		menu_theme_source.set_direct_channels(true);
		menu_theme_source.seek_to(static_cast<float>(settings.start_menu_music_at_secs));
		menu_theme_source.set_gain(0.f);
		menu_theme_source.play();
	}

	latest_news_query = std::thread([&]() {
		auto result = augs::http_get_request(settings.latest_news_url);
		const std::string delim = "newsbegin";

		const auto it = result.find(delim);

		if (it == std::string::npos) {
			return;
		}

		result = result.substr(it + delim.length());

		str_ops(result)
			.multi_replace_all({ "\r", "\n" }, "")
		;

		if (result.size() > 0) {
			const auto wresult = to_wstring(result);

			std::unique_lock<std::mutex> lck(news_mut);
			downloaded_news_text = wresult;
		}
	});

	// TODO: actually load a cosmos with its resources from a file/folder
	const bool is_intro_scene_available = settings.menu_intro_scene_cosmos_path.string().size() > 0;

	if (is_intro_scene_available) {
		intro_scene.set_fixed_delta(60);
#if BUILD_TEST_SCENES
		intro_scene.reserve_storage_for_entities(3000u);

		populate_test_scene_assets(logical_assets, viewable_defs);

		test_scenes::testbed().populate_world_with_entities(
			intro_scene,
			logical_assets,
			[](auto...) {}
		);
#endif
	}

	viewed_character_id = is_intro_scene_available ?
		intro_scene.get_entity_by_name(L"player0")
		: entity_id()
	;

	// director.load_recording_from_file(settings.menu_intro_scene_entropy_path);

	const bool is_recording_available = is_intro_scene_available && director.is_recording_available();
	initial_step_number = intro_scene.get_total_steps_passed();

	timer.reset_timer();

	for (auto& m : gui.root.buttons) {
		m.hover_highlight_maximum_distance = 10.f;
		m.hover_highlight_duration_ms = 300.f;
	}

	gui.root.buttons[main_menu_button_type::CONNECT_TO_OFFICIAL_UNIVERSE].set_appearing_caption(L"Login to official universe");
	gui.root.buttons[main_menu_button_type::BROWSE_UNOFFICIAL_UNIVERSES].set_appearing_caption(L"Browse unofficial universes");
	gui.root.buttons[main_menu_button_type::HOST_UNIVERSE].set_appearing_caption(L"Host universe");
	gui.root.buttons[main_menu_button_type::CONNECT_TO_UNIVERSE].set_appearing_caption(L"Connect to universe");
	gui.root.buttons[main_menu_button_type::LOCAL_UNIVERSE].set_appearing_caption(L"Local universe");
	gui.root.buttons[main_menu_button_type::EDITOR].set_appearing_caption(L"Editor");
	gui.root.buttons[main_menu_button_type::SETTINGS].set_appearing_caption(L"Settings");
	gui.root.buttons[main_menu_button_type::CREATORS].set_appearing_caption(L"Founders");
	gui.root.buttons[main_menu_button_type::QUIT].set_appearing_caption(L"Quit");

	if (is_recording_available) {
		while (intro_scene.get_total_time_passed_in_seconds() < settings.rewind_intro_scene_by_secs) {
			const auto entropy = cosmic_entropy(director.get_entropy_for_step(intro_scene.get_total_steps_passed() - initial_step_number), intro_scene);

			intro_scene.advance(
				{ entropy, logical_assets },
				[](auto) {},
				[](auto) {}
			);
		}
	}
}

void main_menu_setup::draw_overlays(
	const augs::drawer_with_default output,
	const necessary_images_in_atlas& necessarys,
	const augs::baked_font& gui_font,
	const vec2i screen_size
) const {
	const auto game_logo = necessarys.at(assets::necessary_image_id::MENU_GAME_LOGO);
	const auto game_logo_size = game_logo.get_size();

	ltrb game_logo_rect;
	game_logo_rect.set_position({ screen_size.x / 2.f - game_logo_size.x / 2.f, 50.f });
	game_logo_rect.set_size(game_logo_size);

	output.aabb(game_logo, game_logo_rect);
}