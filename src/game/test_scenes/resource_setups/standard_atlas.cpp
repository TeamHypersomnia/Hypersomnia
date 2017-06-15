#include "generated/setting_build_test_scenes.h"
#if BUILD_TEST_SCENES
#include "all.h"
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
		auto& in = output[game_image_id::TEST_CROSSHAIR];
		in.texture_maps[DIFFUSE] = { "resources/gfx/crosshair.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::TEST_BACKGROUND];
		in.texture_maps[DIFFUSE] = { "resources/gfx/snow_textures/snow3.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::CRATE];
		in.texture_maps[DIFFUSE] = { "resources/gfx/crate2.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::TEST_CROSSHAIR];
		in.texture_maps[DIFFUSE] = { "resources/gfx/crosshair.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::CRATE];
		in.texture_maps[DIFFUSE] = { "resources/gfx/crate2.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::CRATE_DESTROYED];
		in.texture_maps[DIFFUSE] = { "resources/gfx/crate2_destroyed.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::CAR_INSIDE];
		in.texture_maps[DIFFUSE] = { "resources/gfx/crate2.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::CAR_FRONT];
		in.texture_maps[DIFFUSE] = { "resources/gfx/crate2.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::JMIX114];
		in.texture_maps[DIFFUSE] = { "resources/gfx/jmix114.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/jmix114.png", GAME_WORLD_ATLAS };
		in.polygonization_filename = "generated/polygonizations_of_images/jmix114.points";
	}

	{
		auto& in = output[game_image_id::TRUCK_FRONT];
		in.texture_maps[DIFFUSE] = { "resources/gfx/truck_front.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/truck_front.png", GAME_WORLD_ATLAS };
		in.polygonization_filename = "generated/polygonizations_of_images/truck_front.points";
	}

	{
		auto& in = output[game_image_id::TRUCK_INSIDE];
		in.texture_maps[DIFFUSE] = { "resources/gfx/truck_inside.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/truck_inside.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::MENU_GAME_LOGO];
		in.texture_maps[DIFFUSE] = { "resources/gfx/menu_game_logo.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::ASSAULT_RIFLE];
		in.texture_maps[DIFFUSE] = { "resources/gfx/assault_rifle.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/assault_rifle.png", GAME_WORLD_ATLAS };
		in.settings.gui.flip_horizontally = true;
		in.settings.gui.flip_vertically = true;
	}

	{
		auto& in = output[game_image_id::BILMER2000];
		in.texture_maps[DIFFUSE] = { "resources/gfx/bilmer2000.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/bilmer2000.png", GAME_WORLD_ATLAS };
		in.settings.gui.flip_horizontally = true;
		in.settings.gui.flip_vertically = false;
	}

	{
		auto& in = output[game_image_id::KEK9];
		in.texture_maps[DIFFUSE] = { "resources/gfx/kek9.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/kek9.png", GAME_WORLD_ATLAS };
		in.settings.gui.flip_horizontally = true;
		in.settings.gui.flip_vertically = false;
	}

	{
		auto& in = output[game_image_id::SUBMACHINE];
		in.texture_maps[DIFFUSE] = { "resources/gfx/submachine.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/submachine.png", GAME_WORLD_ATLAS };
		in.settings.gui.flip_horizontally = true;
		in.settings.gui.flip_vertically = true;
	}

	{
		auto& in = output[game_image_id::URBAN_CYAN_MACHETE];
		in.texture_maps[DIFFUSE] = { "resources/gfx/urban_cyan_machete.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/urban_cyan_machete.png", GAME_WORLD_ATLAS };
		in.settings.gui.flip_horizontally = true;
		in.settings.gui.flip_vertically = true;
	}

	{
		auto& in = output[game_image_id::ROCKET_LAUNCHER];
		in.texture_maps[DIFFUSE] = { "resources/gfx/rocket_launcher.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/rocket_launcher.png", GAME_WORLD_ATLAS };
		in.settings.gui.flip_horizontally = true;
	}

	{
		auto& in = output[game_image_id::AMPLIFIER_ARM];
		in.texture_maps[DIFFUSE] = { "resources/gfx/amplifier_arm.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/amplifier_arm.png", GAME_WORLD_ATLAS };
		in.settings.gui.flip_horizontally = true;
		in.settings.gui.flip_vertically = false;
	}

	{
		auto& in = output[game_image_id::SAMPLE_MAGAZINE];
		in.texture_maps[DIFFUSE] = { "resources/gfx/magazine.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/magazine.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::SMALL_MAGAZINE];
		in.texture_maps[DIFFUSE] = { "resources/gfx/small_magazine.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/small_magazine.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::SAMPLE_SUPPRESSOR];
		in.texture_maps[DIFFUSE] = { "resources/gfx/suppressor.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/suppressor.png", GAME_WORLD_ATLAS };
		in.settings.gui.flip_horizontally = true;
	}

	{
		auto& in = output[game_image_id::ROUND_TRACE];
		in.texture_maps[DIFFUSE] = { "resources/gfx/round_trace.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/round_trace.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::ENERGY_BALL];
		in.texture_maps[DIFFUSE] = { "resources/gfx/energy_ball.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/energy_ball.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::PINK_CHARGE];
		in.texture_maps[DIFFUSE] = { "resources/gfx/pink_charge.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/pink_charge.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::PINK_SHELL];
		in.texture_maps[DIFFUSE] = { "resources/gfx/pink_shell.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/pink_shell.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::CYAN_CHARGE];
		in.texture_maps[DIFFUSE] = { "resources/gfx/cyan_charge.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/cyan_charge.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::CYAN_SHELL];
		in.texture_maps[DIFFUSE] = { "resources/gfx/cyan_shell.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/cyan_shell.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::RED_CHARGE];
		in.texture_maps[DIFFUSE] = { "resources/gfx/red_charge.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/red_charge.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::RED_SHELL];
		in.texture_maps[DIFFUSE] = { "resources/gfx/red_shell.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/red_shell.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::GREEN_CHARGE];
		in.texture_maps[DIFFUSE] = { "resources/gfx/green_charge.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/green_charge.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::GREEN_SHELL];
		in.texture_maps[DIFFUSE] = { "resources/gfx/green_shell.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/green_shell.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::BACKPACK];
		in.texture_maps[DIFFUSE] = { "resources/gfx/backpack.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/backpack.png", GAME_WORLD_ATLAS };
		in.polygonization_filename = "generated/polygonizations_of_images/backpack.points";
		in.settings.gui.bbox_expander = vec2(0, 2);
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
		auto& in = output[game_image_id::HAVE_A_PLEASANT];
		in.texture_maps[DIFFUSE] = { "resources/gfx/have_a_pleasant.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/have_a_pleasant.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::AWAKENING];
		in.texture_maps[DIFFUSE] = { "resources/gfx/awakening.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/awakening.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::METROPOLIS];
		in.texture_maps[DIFFUSE] = { "resources/gfx/metropolis.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/metropolis.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::BRICK_WALL];
		in.texture_maps[DIFFUSE] = { "resources/gfx/brick_wall.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::ROAD];
		in.texture_maps[DIFFUSE] = { "resources/gfx/road.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/road.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::ROAD_FRONT_DIRT];
		in.texture_maps[DIFFUSE] = { "resources/gfx/road_front_dirt.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/road_front_dirt.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::WANDERING_CROSS];
		in.texture_maps[DIFFUSE] = { "resources/gfx/wandering_cross.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::TRUCK_ENGINE];
		in.texture_maps[DIFFUSE] = { "resources/gfx/truck_engine.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/truck_engine.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::HEALTH_ICON];
		in.texture_maps[DIFFUSE] = { "resources/gfx/health_icon.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::PERSONAL_ELECTRICITY_ICON];
		in.texture_maps[DIFFUSE] = { "resources/gfx/personal_electricity_icon.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::CONSCIOUSNESS_ICON];
		in.texture_maps[DIFFUSE] = { "resources/gfx/consciousness_icon.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::PERK_HASTE_ICON];
		in.texture_maps[DIFFUSE] = { "resources/gfx/perk_haste_icon.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::SPELL_HASTE_ICON];
		in.texture_maps[DIFFUSE] = { "resources/gfx/spell_haste_icon.png", GAME_WORLD_ATLAS };
		in.texture_maps[DESATURATED] = { "generated/desaturations/spell_haste_icon.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::SPELL_FURY_OF_THE_AEONS_ICON];
		in.texture_maps[DIFFUSE] = { "resources/gfx/spell_fury_of_the_aeons_icon.png", GAME_WORLD_ATLAS };
		in.texture_maps[DESATURATED] = { "generated/desaturations/spell_fury_of_the_aeons_icon.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::SPELL_ULTIMATE_WRATH_OF_THE_AEONS_ICON];
		in.texture_maps[DIFFUSE] = { "resources/gfx/spell_ultimate_wrath_of_the_aeons_icon.png", GAME_WORLD_ATLAS };
		in.texture_maps[DESATURATED] = { "generated/desaturations/spell_ultimate_wrath_of_the_aeons_icon.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::SPELL_ELECTRIC_TRIAD_ICON];
		in.texture_maps[DIFFUSE] = { "resources/gfx/spell_electric_triad_icon.png", GAME_WORLD_ATLAS };
		in.texture_maps[DESATURATED] = { "generated/desaturations/spell_electric_triad_icon.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::SPELL_ELECTRIC_SHIELD_ICON];
		in.texture_maps[DIFFUSE] = { "resources/gfx/spell_electric_shield_icon.png", GAME_WORLD_ATLAS };
		in.texture_maps[DESATURATED] = { "generated/desaturations/spell_electric_shield_icon.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::SPELL_BORDER];
		in.texture_maps[DIFFUSE] = { "resources/gfx/spell_border.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::PERK_ELECTRIC_SHIELD_ICON];
		in.texture_maps[DIFFUSE] = { "resources/gfx/perk_electric_shield_icon.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::CAST_HIGHLIGHT];
		in.texture_maps[DIFFUSE] = { "resources/gfx/cast_highlight.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::FORCE_GRENADE];
		in.texture_maps[DIFFUSE] = { "resources/gfx/force_grenade.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/force_grenade.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::PED_GRENADE];
		in.texture_maps[DIFFUSE] = { "resources/gfx/ped_grenade.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/ped_grenade.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::INTERFERENCE_GRENADE];
		in.texture_maps[DIFFUSE] = { "resources/gfx/interference_grenade.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/interference_grenade.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::FORCE_GRENADE_RELEASED];
		in.texture_maps[DIFFUSE] = { "resources/gfx/force_grenade_released.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/force_grenade.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::PED_GRENADE_RELEASED];
		in.texture_maps[DIFFUSE] = { "resources/gfx/ped_grenade_released.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/ped_grenade.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::INTERFERENCE_GRENADE_RELEASED];
		in.texture_maps[DIFFUSE] = { "resources/gfx/interference_grenade_released.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/interference_grenade.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::FORCE_ROCKET];
		in.texture_maps[DIFFUSE] = { "resources/gfx/force_rocket.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/force_rocket.png", GAME_WORLD_ATLAS };
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

	{
		auto& in = output[game_image_id::STANDARD_HEAD];
		in.texture_maps[DIFFUSE] = { "resources/gfx/standard_head.png", GAME_WORLD_ATLAS };
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
		game_image_id::METROPOLIS_TILE_FIRST,
		game_image_id::METROPOLIS_TILE_LAST,
		"resources/gfx/tileset/tile_%x.png"
	);

	make_indexed(
		game_image_id::SMOKE_PARTICLE_FIRST,
		game_image_id(int(game_image_id::SMOKE_PARTICLE_FIRST) + 3),
		"resources/gfx/smoke_%x.png"
	);

	make_indexed(
		game_image_id::PIXEL_THUNDER_FIRST,
		game_image_id::PIXEL_THUNDER_LAST,
		"resources/gfx/pixel_thunder_%x.png",
		"generated/neon_maps/pixel_thunder_%x.png"
	);

	make_indexed(
		game_image_id::BLINK_FIRST,
		game_image_id::BLINK_LAST,
		"resources/gfx/blink_%x.png"
	);

	make_indexed(
		game_image_id::CAST_BLINK_FIRST,
		game_image_id::CAST_BLINK_LAST,
		"resources/gfx/cast_blink_%x.png",
		"generated/neon_maps/cast_blink_%x.png"
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