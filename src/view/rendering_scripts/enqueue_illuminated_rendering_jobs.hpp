#pragma once
#include "view/rendering_scripts/draw_character_glow.h"

void enqueue_illuminated_rendering_jobs(
	augs::thread_pool& pool, 
	const illuminated_rendering_input& in
) {
	using D = augs::dedicated_buffer;
	using DV = augs::dedicated_buffer_vector;

	const auto& necessarys = in.necessary_images;

	const auto& special_indicators = in.special_indicators;

	const auto& cone = in.camera.cone;
	const auto& av = in.audiovisuals;
	const auto& thunders = av.get<thunder_system>();
	const auto& exploding_rings = av.get<exploding_ring_system>();
	const auto& screen_size = cone.screen_size;
	const auto& settings = in.drawing;
	const auto& queried_cone = in.queried_cone;
	const auto& visible = in.all_visible;
	const auto& interp = av.template get<interpolation_system>();
	const auto& gui_font = in.gui_font;
	const auto& game_images = in.game_images;

	const auto& viewed_character = in.camera.viewed_character;
	const auto viewed_character_transform = viewed_character ? viewed_character.find_viewing_transform(interp) : std::optional<transformr>();
	(void)viewed_character_transform;

	const auto& cosm = viewed_character.get_cosmos();
	const auto& indicator_meta = in.indicator_meta;

#if BUILD_STENCIL_BUFFER
	const bool fog_of_war_effective = 
		viewed_character_transform != std::nullopt 
		&& settings.fog_of_war.is_enabled()
	;
#else
	const bool fog_of_war_effective = false;
#endif

	const auto fog_of_war_character_id = fog_of_war_effective ? viewed_character : std::optional<entity_id>();
	const auto cast_highlight_tex = necessarys.at(assets::necessary_image_id::CAST_HIGHLIGHT);

	auto& dedicated = in.renderer.dedicated;

	const auto global_time_seconds = cosm.get_total_seconds_passed(in.interpolation_ratio);

	auto get_drawer_for = [&](const D d) {
		return augs::drawer_with_default { dedicated[d].triangles, necessarys.at(assets::necessary_image_id::BLANK) };
	};

	auto make_drawing_input = [get_drawer_for, &game_images, global_time_seconds, &av, &interp, queried_cone](const D d) {
		return draw_renderable_input { 
			{
				get_drawer_for(d), 
				game_images, 
				global_time_seconds,
				flip_flags(),
				av.randomizing,
				queried_cone
			},
			interp
		};
	};

	auto get_line_drawer_for = [&](const D d) {
		return augs::line_drawer_with_default { dedicated[d].lines, necessarys.at(assets::necessary_image_id::BLANK) };
	};

	auto sentience_hud_job = [&cosm, cone, global_time_seconds, &settings, &necessarys, &dedicated, queried_cone, &visible, viewed_character, &interp, &gui_font, &indicator_meta]() {
		augs::constant_size_vector<requested_sentience_meter, 3> requested_meters;

		std::array<assets::necessary_image_id, 3> circles = {
			assets::necessary_image_id::CIRCULAR_BAR_LARGE,
			assets::necessary_image_id::CIRCULAR_BAR_UNDER_LARGE,
			assets::necessary_image_id::CIRCULAR_BAR_UNDER_UNDER_LARGE
		};

		int current_circle = 0;

		auto& target_vectors = dedicated[DV::SENTIENCE_HUDS];
		target_vectors.resize(3);

		if (settings.draw_hp_bar) {
			requested_meters.push_back({ 
				necessarys.at(circles[current_circle++]),
				meter_id::of<health_meter_instance>(),

				target_vectors[0]
			});
		}

		if (settings.draw_pe_bar) {
			requested_meters.push_back({
				necessarys.at(circles[current_circle++]),
				meter_id::of<personal_electricity_meter_instance>(),

				target_vectors[1]
			});
		}

		if (settings.draw_cp_bar) {
			requested_meters.push_back({
				necessarys.at(circles[current_circle++]),
				meter_id::of<consciousness_meter_instance>(),

				target_vectors[2]
			});
		}

		real32 color_indicator_angle = 45.f;

		if (!settings.draw_nicknames) {
			color_indicator_angle = 0.f;
		}

		auto sentience_queried_cone = queried_cone;
		sentience_queried_cone.eye.zoom *= 0.8f;

		auto& nicknames_output = dedicated[D::NICKNAMES].triangles;
		auto& health_numbers_output = dedicated[D::HEALTH_NUMBERS].triangles;
		auto& indicators_output = dedicated[D::SENTIENCE_INDICATORS].triangles;

		const auto input = draw_sentiences_hud_input {
			nicknames_output,
			health_numbers_output,
			indicators_output,
			cone,
			sentience_queried_cone,
			visible,
			settings,
			cosm,
			viewed_character,
			interp,
			global_time_seconds,
			gui_font,
			requested_meters,

			necessarys.at(assets::necessary_image_id::BIG_COLOR_INDICATOR),
			necessarys.at(assets::necessary_image_id::DANGER_INDICATOR),
			necessarys.at(assets::necessary_image_id::DEATH_INDICATOR),
			necessarys.at(assets::necessary_image_id::BOMB_INDICATOR),
			necessarys.at(assets::necessary_image_id::DEFUSING_INDICATOR),

			color_indicator_angle,
			indicator_meta
		};

		draw_sentiences_hud(input);
	};

	auto explosives_hud_job = [&dedicated, global_time_seconds, &necessarys, &settings, &interp, &cosm, viewed_character]() {
		int current_hud = 0;

		auto& target_vectors = dedicated[DV::EXPLOSIVE_HUDS];
		target_vectors.resize(3);

		requested_explosive_huds requests;

		requests[circular_bar_type::SMALL] = {
			necessarys.at(assets::necessary_image_id::CIRCULAR_BAR_SMALL),
			std::addressof(target_vectors[current_hud++])
		};

		requests[circular_bar_type::MEDIUM] = {
			necessarys.at(assets::necessary_image_id::CIRCULAR_BAR_MEDIUM),
			std::addressof(target_vectors[current_hud++])
		};

		requests[circular_bar_type::OVER_MEDIUM] = {
			necessarys.at(assets::necessary_image_id::CIRCULAR_BAR_OVER_MEDIUM),
			std::addressof(target_vectors[current_hud++])
		};

		draw_circular_progresses({
			settings,
			requests,
			interp,
			cosm,
			viewed_character,
			global_time_seconds
		});
	};

	auto indicators_and_callouts_job = [&interp, viewed_character, &gui_font, &special_indicators, get_drawer_for, get_line_drawer_for, &settings, &cosm, cone, screen_size, &visible]() {
		auto drawer = get_drawer_for(D::INDICATORS_AND_CALLOUTS);
		auto line_drawer = get_line_drawer_for(D::INDICATORS_AND_CALLOUTS);

		if (settings.draw_tactical_indicators.is_enabled) {
			const auto alpha = settings.draw_tactical_indicators.value;

			for (const auto& special : special_indicators) {
				auto col = special.color;
				col.mult_alpha(alpha);

				const auto world_pos = special.transform.pos;
				const auto pos = cone.to_screen_space(world_pos);

				std::string primary_text;

				if (const auto callout = cosm[::get_current_callout(cosm, world_pos)]) {
					primary_text = callout.get_name();
				}

				const auto& tex = special.offscreen_tex;

				::draw_offscreen_indicator(
					drawer,
					settings.draw_offscreen_indicators,
					special.draw_onscreen,
					col,
					tex,
					pos,
					screen_size,
					false,
					std::nullopt,
					gui_font,
					primary_text,
					{},
					cone.to_screen_space(viewed_character.get_viewing_transform(interp).pos),
					settings.offscreen_reference_mode
				);
			}
		}

		{
			const auto& callouts = settings.draw_callout_indicators;

			if (callouts.is_enabled) {
				visible.for_each<render_layer::CALLOUT_MARKERS, render_layer::OVERLAID_CALLOUT_MARKERS>(cosm, [&](const auto e) {
					e.template dispatch_on_having_all<invariants::box_marker>([&](const auto typed_handle) { 
						const auto where = typed_handle.get_logic_transform();
						const auto& callout_alpha = callouts.value;

						::draw_area_indicator(typed_handle, line_drawer, where, callout_alpha, drawn_indicator_type::INGAME);

						using namespace augs::gui::text;

						const auto& callout_name = typed_handle.get_name();

						print_stroked(
							drawer,
							cone.to_screen_space(typed_handle.get_logic_transform().pos),
							formatted_string { callout_name, { gui_font, white } },
							{ augs::ralign::CX, augs::ralign::CY }
						);
					});
				});
			}
		}

		if (settings.print_current_character_callout) {
			if (const auto result = cosm[::get_current_callout(viewed_character, interp)]) {
				const auto& name = result.get_name();

				using namespace augs::gui::text;

				const auto indicator_pos = augs::get_screen_pos_from_offset(screen_size, settings.radar_pos); 

				print_stroked(
					drawer,
					indicator_pos,
					formatted_string { name, { gui_font, yellow } }
				);
			}
		}
	};

	auto enqueue_layer_jobs = [&]() {
		auto make_helper = [&](const D d) {
			return helper_drawer {
				visible,
				cosm,
				make_drawing_input(d)
			};
		};

		{
			auto job = [h1 = make_helper(D::WALL_LIGHTED_BODIES), h2 = make_helper(D::OVER_SENTIENCES), h3 = make_helper(D::NEON_OCCLUDING_DYNAMIC_BODY)]() {
				h1.draw<
					render_layer::DYNAMIC_BODY,
					render_layer::OVER_DYNAMIC_BODY
				>();

				h2.draw<
					render_layer::OVER_SENTIENCES
				>();

				h3.draw<
					render_layer::NEON_OCCLUDING_DYNAMIC_BODY
				>();
			};

			pool.enqueue(job);
		}

		{
			auto job = [h = make_helper(D::DECORATION_NEONS)]() {
				h.draw_neons<
					render_layer::FLYING_BULLETS,
					render_layer::WATER_COLOR_OVERLAYS,
					render_layer::WATER_SURFACES,
					render_layer::NEON_CAPTIONS,
					render_layer::PLANTED_BOMBS,
					render_layer::AQUARIUM_FLOWERS,
					render_layer::BOTTOM_FISH,
					render_layer::UPPER_FISH,
					render_layer::INSECTS,
					render_layer::AQUARIUM_BUBBLES
				>();
			};

			pool.enqueue(job);
		}

		{
			auto job = [h = make_helper(D::BODY_NEONS)]() {
				h.draw_neons<
					render_layer::DYNAMIC_BODY,
					render_layer::OVER_DYNAMIC_BODY,
					render_layer::GLASS_BODY,
					render_layer::SMALL_DYNAMIC_BODY,
					render_layer::OVER_SMALL_DYNAMIC_BODY,
					render_layer::OVER_SENTIENCES,
					render_layer::NEON_OCCLUDING_DYNAMIC_BODY
				>();
			};

			pool.enqueue(job);
		}

		{
			auto job = [h1 = make_helper(D::FLOOR_NEONS), h2 = make_helper(D::FLOOR_NEON_OVERLAYS)]() {
				h1.draw_neons<
					render_layer::GROUND
				>();

				h2.draw<
					render_layer::GROUND_NEON_OVERLAY
				>();
			};

			pool.enqueue(job);
		}

		{
			auto job = [h = make_helper(D::SMALL_AND_GLASS_BODIES)]() {
				h.draw<
					render_layer::SMALL_DYNAMIC_BODY,
					render_layer::OVER_SMALL_DYNAMIC_BODY,
					render_layer::GLASS_BODY
				>();
			};

			pool.enqueue(job);
		}

		{
			auto job = [h = make_helper(D::GROUND_AND_DECORS)]() {
				h.draw<
					render_layer::GROUND,

					render_layer::PLANTED_BOMBS,

					render_layer::AQUARIUM_FLOWERS,
					render_layer::AQUARIUM_DUNES,
					render_layer::BOTTOM_FISH,
					render_layer::UPPER_FISH,
					render_layer::AQUARIUM_BUBBLES
				>();
			};

			pool.enqueue(job);
		}

		{
			auto job = [h1 = make_helper(D::WATER_AND_CARS), h2 = make_helper(D::INSECTS), h3 = make_helper(D::CAPTIONS_AND_BULLETS)]() {
				h1.draw<
					render_layer::WATER_COLOR_OVERLAYS,
					render_layer::WATER_SURFACES
				>();

				h2.draw<
					render_layer::INSECTS
				>();

				h3.draw<
					render_layer::FLYING_BULLETS,
					render_layer::NEON_CAPTIONS
				>();
			};

			pool.enqueue(job);
		}
	};

	auto sentiences_job = [cast_highlight_tex, &cosm, fog_of_war_character_id, make_drawing_input, &visible, &interp, global_time_seconds]() {
		auto draw_lights_for = [&](const auto& drawing_in, const auto& handle) {
			::specific_draw_neon_map(handle, drawing_in);
			::draw_character_glow(
				handle, 
				{
					drawing_in.drawer,
					interp,
					global_time_seconds,
					cast_highlight_tex
				}
			);
		};

		const auto timestamp_ms = static_cast<unsigned>(global_time_seconds * 1000);

		auto standard_border_provider = [timestamp_ms](const auto& typed_handle) {
			return typed_handle.template get<components::sentience>().find_low_health_border(timestamp_ms);
		};

		const auto friendly_drawing_in = make_drawing_input(D::FRIENDLY_SENTIENCES);
		const auto enemy_drawing_in = make_drawing_input(D::ENEMY_SENTIENCES);

		const auto borders_friendly_drawing_in = make_drawing_input(D::BORDERS_FRIENDLY_SENTIENCES);
		const auto borders_enemy_drawing_in = make_drawing_input(D::BORDERS_ENEMY_SENTIENCES);

		const auto neons_friendly_drawing_in = make_drawing_input(D::NEONS_FRIENDLY_SENTIENCES);
		const auto neons_enemy_drawing_in = make_drawing_input(D::NEONS_ENEMY_SENTIENCES);

		if (const auto fog_of_war_character = cosm[fog_of_war_character_id ? *fog_of_war_character_id : entity_id()]) {
			const auto fow_faction = fog_of_war_character.get_official_faction();

			visible.for_each<render_layer::SENTIENCES>(cosm, [&](const auto& handle) {
				handle.template dispatch_on_having_all<components::sentience>([&](const auto& typed_handle) {
					if (typed_handle.get_official_faction() == fow_faction) {
						draw_lights_for(neons_friendly_drawing_in, typed_handle);

						::specific_draw_entity(typed_handle, friendly_drawing_in);
						::specific_draw_border(typed_handle, borders_friendly_drawing_in, standard_border_provider);
					}
					else {
						draw_lights_for(neons_enemy_drawing_in, typed_handle);

						::specific_draw_entity(typed_handle, enemy_drawing_in);
						::specific_draw_border(typed_handle, borders_enemy_drawing_in, standard_border_provider);
					}
				});
			});
		}
		else {
			visible.for_each<render_layer::SENTIENCES>(cosm, [&](const auto& handle) {
				handle.template dispatch_on_having_all<components::sentience>([&](const auto& typed_handle) {
					draw_lights_for(neons_friendly_drawing_in, typed_handle);

					::specific_draw_entity(typed_handle, friendly_drawing_in);
					::specific_draw_border(typed_handle, borders_friendly_drawing_in, standard_border_provider);
				});
			});
		}
	};

	auto special_effects_job = [&exploding_rings, &dedicated, get_drawer_for, &thunders, queried_cone, get_line_drawer_for]() {
		{
			thunders.draw_thunders(
				get_line_drawer_for(D::THUNDERS),
				queried_cone
			);
		}

		{
			exploding_rings.draw_rings(
				get_drawer_for(D::EXPLODING_RINGS),
				dedicated[D::EXPLODING_RINGS].specials,
				queried_cone
			);
		}
	};

	enqueue_layer_jobs();

	pool.enqueue(sentiences_job);

	if (viewed_character) {
		pool.enqueue(indicators_and_callouts_job);
		pool.enqueue(sentience_hud_job);
	}

	pool.enqueue(explosives_hud_job);
	pool.enqueue(special_effects_job);
}

