#include "generated/setting_build_test_scenes.h"
#if BUILD_TEST_SCENES
#include "game/hardcoded_content/all_hardcoded_content.h"
#include "game/assets/assets_manager.h"
#include "augs/graphics/shader.h"
#include "augs/gui/button_corners.h"

using namespace assets;

static constexpr auto DIFFUSE = texture_map_type::DIFFUSE;
static constexpr auto NEON = texture_map_type::NEON;
static constexpr auto DESATURATED = texture_map_type::DESATURATED;
static constexpr auto GAME_WORLD_ATLAS = gl_texture_id::GAME_WORLD_ATLAS;

game_image_requests load_requisite_images() {
	game_image_requests output;

	const auto make_button_with_corners = [&](
		const game_image_id first,
		const std::string& filename_template,
		const bool request_lb_complement
	) {
		const auto first_i = static_cast<int>(first);
		const auto last_i = first_i + static_cast<int>(button_corner_type::COUNT);

		for (int i = first_i; i < last_i; ++i) {
			const auto type = static_cast<button_corner_type>(i - first_i);

			if (!request_lb_complement && is_lb_complement(type)) {
				continue;
			}

			const auto full_filename = typesafe_sprintf(filename_template, get_filename_for(type));

			{
				auto& in = output[static_cast<game_image_id>(i)];
				in.texture_maps[DIFFUSE] = { full_filename, GAME_WORLD_ATLAS };
			}
		}
	};

	const auto make_indexed = [&](
		const game_image_id first,
		const game_image_id last,
		const std::string& filename_template,
		const std::string& neon_filename_template = std::string()
	) {
		const auto first_i = static_cast<int>(first);
		const auto last_i = static_cast<int>(last);

		for (int i = first_i; i < last_i; ++i) {
			auto& in = output[static_cast<game_image_id>(i)];
			in.texture_maps[DIFFUSE] = { typesafe_sprintf(filename_template, 1 + i - first_i), GAME_WORLD_ATLAS };

			if (neon_filename_template.size() > 0) {
				in.texture_maps[NEON] = { typesafe_sprintf(neon_filename_template, 1 + i - first_i), GAME_WORLD_ATLAS };
			}
		}
	};

	{
		auto& in = output[game_image_id::MENU_GAME_LOGO];
		in.texture_maps[DIFFUSE] = { "resources/gfx/menu_game_logo.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::ATTACHMENT_CIRCLE_FILLED];
		in.texture_maps[DIFFUSE] = { "generated/scripted_images/attachment_circle_filled.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::ATTACHMENT_CIRCLE_BORDER];
		in.texture_maps[DIFFUSE] = { "generated/scripted_images/attachment_circle_border.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::ACTION_BUTTON_FILLED];
		in.texture_maps[DIFFUSE] = { "generated/scripted_images/action_button_filled.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::ACTION_BUTTON_BORDER];
		in.texture_maps[DIFFUSE] = { "generated/scripted_images/action_button_border.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::HUD_CIRCULAR_BAR_MEDIUM];
		in.texture_maps[DIFFUSE] = { "generated/scripted_images/circular_bar_medium.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::HUD_CIRCULAR_BAR_SMALL];
		in.texture_maps[DIFFUSE] = { "generated/scripted_images/circular_bar_small.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::WANDERING_CROSS];
		in.texture_maps[DIFFUSE] = { "resources/gfx/wandering_cross.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::SPELL_BORDER];
		in.texture_maps[DIFFUSE] = { "resources/gfx/spell_border.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::CAST_HIGHLIGHT];
		in.texture_maps[DIFFUSE] = { "resources/gfx/cast_highlight.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::CONTAINER_OPEN_ICON];
		in.texture_maps[DIFFUSE] = { "resources/gfx/container_open_icon.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::CONTAINER_CLOSED_ICON];
		in.texture_maps[DIFFUSE] = { "resources/gfx/container_closed_icon.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::GUI_CURSOR];
		in.texture_maps[DIFFUSE] = { "resources/gfx/gui_cursor.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::GUI_CURSOR_HOVER];
		in.texture_maps[DIFFUSE] = { "resources/gfx/gui_cursor_hover.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::GUI_CURSOR_ADD];
		in.texture_maps[DIFFUSE] = { "resources/gfx/gui_cursor_add.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::GUI_CURSOR_ERROR];
		in.texture_maps[DIFFUSE] = { "resources/gfx/gui_cursor_error.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::GUI_CURSOR_MINUS];
		in.texture_maps[DIFFUSE] = { "resources/gfx/gui_cursor_minus.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::BLANK];
		in.texture_maps[DIFFUSE] = { "resources/gfx/blank.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::LASER];
		in.texture_maps[DIFFUSE] = { "resources/gfx/laser.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "resources/gfx/laser_neon_map.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::LASER_GLOW_EDGE];
		in.texture_maps[DIFFUSE] = { "resources/gfx/laser_glow_edge.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::DROP_HAND_ICON];
		in.texture_maps[DIFFUSE] = { "resources/gfx/drop_hand_icon.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::SECONDARY_HAND_ICON];
		in.texture_maps[DIFFUSE] = { "resources/gfx/secondary_hand_icon.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::PRIMARY_HAND_ICON];
		in.texture_maps[DIFFUSE] = { "resources/gfx/primary_hand_icon.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::SHOULDER_SLOT_ICON];
		in.texture_maps[DIFFUSE] = { "resources/gfx/shoulder_slot_icon.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::ARMOR_SLOT_ICON];
		in.texture_maps[DIFFUSE] = { "resources/gfx/armor_slot_icon.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::CHAMBER_SLOT_ICON];
		in.texture_maps[DIFFUSE] = { "resources/gfx/chamber_slot_icon.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::DETACHABLE_MAGAZINE_ICON];
		in.texture_maps[DIFFUSE] = { "resources/gfx/detachable_magazine_slot_icon.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::GUN_MUZZLE_SLOT_ICON];
		in.texture_maps[DIFFUSE] = { "resources/gfx/gun_muzzle_slot_icon.png", GAME_WORLD_ATLAS };
	}

	make_button_with_corners(
		game_image_id::HOTBAR_BUTTON_INSIDE,
		"generated/buttons_with_corners/hotbar_button_%x.png",
		true
	);

	make_button_with_corners(
		game_image_id::MENU_BUTTON_INSIDE,
		"generated/buttons_with_corners/menu_button_%x.png",
		false
	);

	make_indexed(
		game_image_id::BLINK_FIRST,
		game_image_id::BLINK_LAST,
		"resources/gfx/blink_%x.png"
	);

	return output;
}

game_font_requests load_requisite_fonts() {
	game_font_requests output;

	{
		game_font_request in;
		in.target_atlas = GAME_WORLD_ATLAS;
		in.loading_input.path = "resources/fonts/Kubasta.ttf";
		in.loading_input.characters = L" ABCDEFGHIJKLMNOPRSTUVWXYZQabcdefghijklmnoprstuvwxyzq0123456789.!@#$%^&*()_+-=[];'\\,./{}:\"|<>?~";
		in.loading_input.pt = 16;

		output[font_id::GUI_FONT] = in;
	}

	return output;
}
#endif