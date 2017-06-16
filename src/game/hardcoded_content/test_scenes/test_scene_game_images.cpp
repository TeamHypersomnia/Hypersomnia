#include "generated/setting_build_test_scenes.h"
#if BUILD_TEST_SCENES
#include "game/hardcoded_content/all_hardcoded_content.h"
#include "game/hardcoded_content/loader_utils.h"

game_image_requests load_test_scene_images() {
	game_image_requests output;

	{
		auto& in = output[game_image_id::TEST_CROSSHAIR];
		in.texture_maps[DIFFUSE] = { "content/gfx/crosshair.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::TEST_BACKGROUND];
		in.texture_maps[DIFFUSE] = { "content/gfx/snow3.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::CRATE];
		in.texture_maps[DIFFUSE] = { "content/gfx/crate2.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::TEST_CROSSHAIR];
		in.texture_maps[DIFFUSE] = { "content/gfx/crosshair.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::CRATE];
		in.texture_maps[DIFFUSE] = { "content/gfx/crate2.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::CRATE_DESTROYED];
		in.texture_maps[DIFFUSE] = { "content/gfx/crate2_destroyed.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::CAR_INSIDE];
		in.texture_maps[DIFFUSE] = { "content/gfx/crate2.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::CAR_FRONT];
		in.texture_maps[DIFFUSE] = { "content/gfx/crate2.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::JMIX114];
		in.texture_maps[DIFFUSE] = { "content/gfx/jmix114.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/jmix114.png", GAME_WORLD_ATLAS };
		in.polygonization_filename = "generated/polygonizations_of_images/jmix114.points";
	}

	{
		auto& in = output[game_image_id::TRUCK_FRONT];
		in.texture_maps[DIFFUSE] = { "content/gfx/truck_front.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/truck_front.png", GAME_WORLD_ATLAS };
		in.polygonization_filename = "generated/polygonizations_of_images/truck_front.points";
	}

	{
		auto& in = output[game_image_id::TRUCK_INSIDE];
		in.texture_maps[DIFFUSE] = { "content/gfx/truck_inside.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/truck_inside.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::ASSAULT_RIFLE];
		in.texture_maps[DIFFUSE] = { "content/gfx/assault_rifle.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/assault_rifle.png", GAME_WORLD_ATLAS };
		in.settings.gui.flip_horizontally = true;
		in.settings.gui.flip_vertically = true;
	}

	{
		auto& in = output[game_image_id::BILMER2000];
		in.texture_maps[DIFFUSE] = { "content/gfx/bilmer2000.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/bilmer2000.png", GAME_WORLD_ATLAS };
		in.settings.gui.flip_horizontally = true;
		in.settings.gui.flip_vertically = false;
	}

	{
		auto& in = output[game_image_id::KEK9];
		in.texture_maps[DIFFUSE] = { "content/gfx/kek9.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/kek9.png", GAME_WORLD_ATLAS };
		in.settings.gui.flip_horizontally = true;
		in.settings.gui.flip_vertically = false;
	}

	{
		auto& in = output[game_image_id::SUBMACHINE];
		in.texture_maps[DIFFUSE] = { "content/gfx/submachine.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/submachine.png", GAME_WORLD_ATLAS };
		in.settings.gui.flip_horizontally = true;
		in.settings.gui.flip_vertically = true;
	}

	{
		auto& in = output[game_image_id::URBAN_CYAN_MACHETE];
		in.texture_maps[DIFFUSE] = { "content/gfx/urban_cyan_machete.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/urban_cyan_machete.png", GAME_WORLD_ATLAS };
		in.settings.gui.flip_horizontally = true;
		in.settings.gui.flip_vertically = true;
	}

	{
		auto& in = output[game_image_id::ROCKET_LAUNCHER];
		in.texture_maps[DIFFUSE] = { "content/gfx/rocket_launcher.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/rocket_launcher.png", GAME_WORLD_ATLAS };
		in.settings.gui.flip_horizontally = true;
	}

	{
		auto& in = output[game_image_id::AMPLIFIER_ARM];
		in.texture_maps[DIFFUSE] = { "content/gfx/amplifier_arm.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/amplifier_arm.png", GAME_WORLD_ATLAS };
		in.settings.gui.flip_horizontally = true;
		in.settings.gui.flip_vertically = false;
	}

	{
		auto& in = output[game_image_id::SAMPLE_MAGAZINE];
		in.texture_maps[DIFFUSE] = { "content/gfx/magazine.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/magazine.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::SMALL_MAGAZINE];
		in.texture_maps[DIFFUSE] = { "content/gfx/small_magazine.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/small_magazine.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::SAMPLE_SUPPRESSOR];
		in.texture_maps[DIFFUSE] = { "content/gfx/suppressor.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/suppressor.png", GAME_WORLD_ATLAS };
		in.settings.gui.flip_horizontally = true;
	}

	{
		auto& in = output[game_image_id::ROUND_TRACE];
		in.texture_maps[DIFFUSE] = { "content/gfx/round_trace.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/round_trace.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::ENERGY_BALL];
		in.texture_maps[DIFFUSE] = { "content/gfx/energy_ball.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/energy_ball.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::PINK_CHARGE];
		in.texture_maps[DIFFUSE] = { "content/gfx/pink_charge.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/pink_charge.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::PINK_SHELL];
		in.texture_maps[DIFFUSE] = { "content/gfx/pink_shell.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/pink_shell.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::CYAN_CHARGE];
		in.texture_maps[DIFFUSE] = { "content/gfx/cyan_charge.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/cyan_charge.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::CYAN_SHELL];
		in.texture_maps[DIFFUSE] = { "content/gfx/cyan_shell.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/cyan_shell.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::RED_CHARGE];
		in.texture_maps[DIFFUSE] = { "content/gfx/red_charge.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/red_charge.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::RED_SHELL];
		in.texture_maps[DIFFUSE] = { "content/gfx/red_shell.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/red_shell.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::GREEN_CHARGE];
		in.texture_maps[DIFFUSE] = { "content/gfx/green_charge.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/green_charge.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::GREEN_SHELL];
		in.texture_maps[DIFFUSE] = { "content/gfx/green_shell.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/green_shell.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::BACKPACK];
		in.texture_maps[DIFFUSE] = { "content/gfx/backpack.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/backpack.png", GAME_WORLD_ATLAS };
		in.polygonization_filename = "generated/polygonizations_of_images/backpack.points";
		in.settings.gui.bbox_expander = vec2(0, 2);
	}

	{
		auto& in = output[game_image_id::HAVE_A_PLEASANT];
		in.texture_maps[DIFFUSE] = { "content/gfx/have_a_pleasant.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/have_a_pleasant.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::AWAKENING];
		in.texture_maps[DIFFUSE] = { "content/gfx/awakening.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/awakening.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::METROPOLIS];
		in.texture_maps[DIFFUSE] = { "content/gfx/metropolis.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/metropolis.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::BRICK_WALL];
		in.texture_maps[DIFFUSE] = { "content/gfx/brick_wall.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::ROAD];
		in.texture_maps[DIFFUSE] = { "content/gfx/road.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/road.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::ROAD_FRONT_DIRT];
		in.texture_maps[DIFFUSE] = { "content/gfx/road_front_dirt.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/road_front_dirt.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::TRUCK_ENGINE];
		in.texture_maps[DIFFUSE] = { "content/gfx/truck_engine.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/truck_engine.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::HEALTH_ICON];
		in.texture_maps[DIFFUSE] = { "content/gfx/health_icon.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::PERSONAL_ELECTRICITY_ICON];
		in.texture_maps[DIFFUSE] = { "content/gfx/personal_electricity_icon.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::CONSCIOUSNESS_ICON];
		in.texture_maps[DIFFUSE] = { "content/gfx/consciousness_icon.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::PERK_HASTE_ICON];
		in.texture_maps[DIFFUSE] = { "content/gfx/perk_haste_icon.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::SPELL_HASTE_ICON];
		in.texture_maps[DIFFUSE] = { "content/gfx/spell_haste_icon.png", GAME_WORLD_ATLAS };
		in.texture_maps[DESATURATED] = { "generated/desaturations/spell_haste_icon.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::SPELL_FURY_OF_THE_AEONS_ICON];
		in.texture_maps[DIFFUSE] = { "content/gfx/spell_fury_of_the_aeons_icon.png", GAME_WORLD_ATLAS };
		in.texture_maps[DESATURATED] = { "generated/desaturations/spell_fury_of_the_aeons_icon.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::SPELL_ULTIMATE_WRATH_OF_THE_AEONS_ICON];
		in.texture_maps[DIFFUSE] = { "content/gfx/spell_ultimate_wrath_of_the_aeons_icon.png", GAME_WORLD_ATLAS };
		in.texture_maps[DESATURATED] = { "generated/desaturations/spell_ultimate_wrath_of_the_aeons_icon.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::SPELL_ELECTRIC_TRIAD_ICON];
		in.texture_maps[DIFFUSE] = { "content/gfx/spell_electric_triad_icon.png", GAME_WORLD_ATLAS };
		in.texture_maps[DESATURATED] = { "generated/desaturations/spell_electric_triad_icon.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::SPELL_ELECTRIC_SHIELD_ICON];
		in.texture_maps[DIFFUSE] = { "content/gfx/spell_electric_shield_icon.png", GAME_WORLD_ATLAS };
		in.texture_maps[DESATURATED] = { "generated/desaturations/spell_electric_shield_icon.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::PERK_ELECTRIC_SHIELD_ICON];
		in.texture_maps[DIFFUSE] = { "content/gfx/perk_electric_shield_icon.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::FORCE_GRENADE];
		in.texture_maps[DIFFUSE] = { "content/gfx/force_grenade.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/force_grenade.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::PED_GRENADE];
		in.texture_maps[DIFFUSE] = { "content/gfx/ped_grenade.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/ped_grenade.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::INTERFERENCE_GRENADE];
		in.texture_maps[DIFFUSE] = { "content/gfx/interference_grenade.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/interference_grenade.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::FORCE_GRENADE_RELEASED];
		in.texture_maps[DIFFUSE] = { "content/gfx/force_grenade_released.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/force_grenade.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::PED_GRENADE_RELEASED];
		in.texture_maps[DIFFUSE] = { "content/gfx/ped_grenade_released.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/ped_grenade.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::INTERFERENCE_GRENADE_RELEASED];
		in.texture_maps[DIFFUSE] = { "content/gfx/interference_grenade_released.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/interference_grenade.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::FORCE_ROCKET];
		in.texture_maps[DIFFUSE] = { "content/gfx/force_rocket.png", GAME_WORLD_ATLAS };
		in.texture_maps[NEON] = { "generated/neon_maps/force_rocket.png", GAME_WORLD_ATLAS };
	}

	{
		auto& in = output[game_image_id::STANDARD_HEAD];
		in.texture_maps[DIFFUSE] = { "content/gfx/standard_head.png", GAME_WORLD_ATLAS };
	}

	make_indexed_images(
		output,
		game_image_id::METROPOLIS_TILE_FIRST,
		game_image_id::METROPOLIS_TILE_LAST,
		"content/gfx/tileset/tile_%x.png"
	);

	make_indexed_images(
		output,
		game_image_id::SMOKE_PARTICLE_FIRST,
		game_image_id(int(game_image_id::SMOKE_PARTICLE_FIRST) + 3),
		"content/gfx/smoke_%x.png"
	);

	make_indexed_images(
		output,
		game_image_id::PIXEL_THUNDER_FIRST,
		game_image_id::PIXEL_THUNDER_LAST,
		"content/gfx/pixel_thunder_%x.png",
		"generated/neon_maps/pixel_thunder_%x.png"
	);

	make_indexed_images(
		output,
		game_image_id::CAST_BLINK_FIRST,
		game_image_id::CAST_BLINK_LAST,
		"content/gfx/cast_blink_%x.png",
		"generated/neon_maps/cast_blink_%x.png"
	);

	return output;
}
#endif