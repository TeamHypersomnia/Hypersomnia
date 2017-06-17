#include "game/hardcoded_content/all_hardcoded_content.h"
#include "game/hardcoded_content/loader_utils.h"

game_image_requests load_requisite_images() {
	game_image_requests output;

	{
		auto& in = output[game_image_id::MENU_GAME_LOGO];
		in.texture_maps[DIFFUSE] = { "content/gfx/menu_game_logo.png", GAME_WORLD_ATLAS };
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
		in.texture_maps[DIFFUSE] = { "content/gfx/wandering_cross.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::SPELL_BORDER];
		in.texture_maps[DIFFUSE] = { "content/gfx/spell_border.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::CAST_HIGHLIGHT];
		in.texture_maps[DIFFUSE] = { "content/gfx/cast_highlight.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::CONTAINER_OPEN_ICON];
		in.texture_maps[DIFFUSE] = { "content/gfx/container_open_icon.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::CONTAINER_CLOSED_ICON];
		in.texture_maps[DIFFUSE] = { "content/gfx/container_closed_icon.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::GUI_CURSOR];
		in.texture_maps[DIFFUSE] = { "content/gfx/gui_cursor.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::GUI_CURSOR_HOVER];
		in.texture_maps[DIFFUSE] = { "content/gfx/gui_cursor_hover.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::GUI_CURSOR_ADD];
		in.texture_maps[DIFFUSE] = { "content/gfx/gui_cursor_add.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::GUI_CURSOR_ERROR];
		in.texture_maps[DIFFUSE] = { "content/gfx/gui_cursor_error.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::GUI_CURSOR_MINUS];
		in.texture_maps[DIFFUSE] = { "content/gfx/gui_cursor_minus.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::BLANK];
		in.texture_maps[DIFFUSE] = { "content/gfx/blank.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::LASER];
		in.texture_maps[DIFFUSE] = { "content/gfx/laser.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "content/gfx/laser_neon_map.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::LASER_GLOW_EDGE];
		in.texture_maps[DIFFUSE] = { "content/gfx/laser_glow_edge.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::DROP_HAND_ICON];
		in.texture_maps[DIFFUSE] = { "content/gfx/drop_hand_icon.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::SECONDARY_HAND_ICON];
		in.texture_maps[DIFFUSE] = { "content/gfx/secondary_hand_icon.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::PRIMARY_HAND_ICON];
		in.texture_maps[DIFFUSE] = { "content/gfx/primary_hand_icon.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::SHOULDER_SLOT_ICON];
		in.texture_maps[DIFFUSE] = { "content/gfx/shoulder_slot_icon.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::ARMOR_SLOT_ICON];
		in.texture_maps[DIFFUSE] = { "content/gfx/armor_slot_icon.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::CHAMBER_SLOT_ICON];
		in.texture_maps[DIFFUSE] = { "content/gfx/chamber_slot_icon.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::DETACHABLE_MAGAZINE_ICON];
		in.texture_maps[DIFFUSE] = { "content/gfx/detachable_magazine_slot_icon.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::GUN_MUZZLE_SLOT_ICON];
		in.texture_maps[DIFFUSE] = { "content/gfx/gun_muzzle_slot_icon.png", GAME_WORLD_ATLAS };
	}

	make_button_with_corners(
		output,
		game_image_id::HOTBAR_BUTTON_INSIDE,
		"generated/buttons_with_corners/hotbar_button_%x.png",
		true
	);

	make_button_with_corners(
		output,
		game_image_id::MENU_BUTTON_INSIDE,
		"generated/buttons_with_corners/menu_button_%x.png",
		false
	);

	return output;
}

game_font_requests load_requisite_fonts() {
	game_font_requests output;

	{
		game_font_request in;
		in.target_atlas = GAME_WORLD_ATLAS;
		in.loading_input.path = "content/fonts/Kubasta.ttf";
		in.loading_input.characters = L" ABCDEFGHIJKLMNOPRSTUVWXYZQabcdefghijklmnoprstuvwxyzq0123456789.!@#$%^&*()_+-=[];'\\,./{}:\"|<>?~";
		in.loading_input.pt = 16;

		output[font_id::GUI_FONT] = in;
	}

	return output;
}