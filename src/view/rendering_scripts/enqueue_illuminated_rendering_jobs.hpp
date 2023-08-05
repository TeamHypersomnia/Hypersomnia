#pragma once
#include "view/rendering_scripts/draw_character_glow.h"
#include "game/detail/visible_entities.hpp"
#include "view/rendering_scripts/is_reasonably_in_view.hpp"
#include "game/detail/use_interaction_logic.h"

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
	const auto& damage_indication_settings = in.damage_indication;
	const auto& queried_cone = in.queried_cone;
	const auto& actual_cone = in.camera.cone;
	const auto& visible = in.all_visible;
	const auto& interp = av.template get<interpolation_system>();
	const auto& damage_indication = av.template get<damage_indication_system>();
	const auto& gui_font = in.fonts.gui;
	const auto& game_images = in.game_images;
	const auto pre_step_crosshair_displacement = in.pre_step_crosshair_displacement;

	const auto& viewed_character = in.camera.viewed_character;
	const auto potential_interaction = viewed_character.alive() ? ::query_use_interaction(viewed_character) : std::optional<use_interaction_variant>();

	const auto viewed_character_transform = viewed_character ? viewed_character.find_viewing_transform(interp) : std::optional<transformr>();
	(void)viewed_character_transform;

	const auto& cosm = viewed_character.get_cosmos();
	const auto indicator_meta = in.indicator_meta;

#if BUILD_STENCIL_BUFFER
	const bool fog_of_war_effective = 
		viewed_character_transform.has_value() 
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

	auto sentience_hud_job = [&cosm, cone, global_time_seconds, &settings, &necessarys, &dedicated, queried_cone, &visible, viewed_character, &interp, &gui_font, indicator_meta, fog_of_war_effective, pre_step_crosshair_displacement, &damage_indication, damage_indication_settings]() {
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
		auto& small_health_bars_output = dedicated[D::SMALL_HEALTH_BARS].triangles;

		auto is_reasonably_in_view = [&](const const_entity_handle character) {
			if (!fog_of_war_effective) {
				return true;
			}

			return ::is_reasonably_in_view(
				viewed_character,
				character,
				pre_step_crosshair_displacement,
				interp,
				settings.fog_of_war.angle
			);
		};

		const auto input = draw_sentiences_hud_input {
			nicknames_output,
			health_numbers_output,
			indicators_output,
			small_health_bars_output,
			cone,
			sentience_queried_cone,
			visible,
			settings,
			cosm,
			viewed_character,
			interp,
			damage_indication,
			damage_indication_settings,
			global_time_seconds,
			gui_font,
			requested_meters,

			necessarys.at(assets::necessary_image_id::BLANK),
			necessarys.at(assets::necessary_image_id::BIG_COLOR_INDICATOR),
			necessarys.at(assets::necessary_image_id::DANGER_INDICATOR),
			necessarys.at(assets::necessary_image_id::DEATH_INDICATOR),
			necessarys.at(assets::necessary_image_id::BOMB_INDICATOR),
			necessarys.at(assets::necessary_image_id::DEFUSING_INDICATOR),

			color_indicator_angle,
			indicator_meta,
			is_reasonably_in_view
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
				visible.for_each<render_layer::CALLOUT_MARKERS>(cosm, [&](const auto e) {
					e.template dispatch_on_having_all<invariants::area_marker>([&](const auto typed_handle) { 
						const auto where = typed_handle.get_logic_transform();
						const auto& callout_alpha = callouts.value;

						::draw_area_indicator(typed_handle, line_drawer, where, callout_alpha, drawn_indicator_type::INGAME, 1.0f);

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
			auto job = [h1 = make_helper(D::SOLID_OBSTACLES), h2 = make_helper(D::FOREGROUND), h3 = make_helper(D::WALL_ILLUMINATIONS)]() {
				h1.draw<
					render_layer::SOLID_OBSTACLES
				>();

				h2.draw<
					render_layer::FOREGROUND
				>();

				h3.draw<
					special_render_function::ILLUMINATE_AS_WALL
				>();
			};

			pool.enqueue(job);
		}

		{
			auto job = [h1 = make_helper(D::UNDER_FOREGROUND_NEONS), h2 = make_helper(D::FOREGROUND_NEONS)]() {
				h1.draw_neons<
					render_layer::PLANTED_ITEMS,
					render_layer::OBSTACLES_UNDER_MISSILES,
					render_layer::MISSILES,
					render_layer::SOLID_OBSTACLES,
					render_layer::REMNANTS
					/* Sentiences would come here if not for the fact that they have special neon logic */
				>();

				h2.draw_neons<
					render_layer::FOREGROUND,
					render_layer::FOREGROUND_GLOWS
				>();
			};

			pool.enqueue(job);
		}

		{
			auto job = [h1 = make_helper(D::GROUND_NEONS), h2 = make_helper(D::GROUND_NEON_OCCLUDERS), h3 = make_helper(D::FOREGROUND_NEON_OCCLUDERS)]() {
				h1.draw_neons<
					render_layer::GROUND
				>();

				h2.draw<
					special_render_function::COVER_GROUND_NEONS
				>();

				h3.draw<
					special_render_function::COVER_GROUND_NEONS_FOREGROUND
				>();
			};

			pool.enqueue(job);
		}

		{
			auto job = [h = make_helper(D::REMNANTS)]() {
				h.draw<
					render_layer::REMNANTS
				>();
			};

			pool.enqueue(job);
		}

		{
			auto pickable_item_to_highlight = entity_id();
			auto pickable_color = white;

			const auto bad_color = rgba(255, 50, 50, 255);

			if (potential_interaction.has_value()) {
				auto handle_interaction = [&](const auto& typed_interaction) {
					using T = remove_cref<decltype(typed_interaction)>;

					if constexpr(std::is_same_v<T, item_pickup>) {
						const auto item_handle = cosm[typed_interaction.item];
						const auto pickup_slot = viewed_character.find_pickup_target_slot_for(item_handle, { slot_finding_opt::OMIT_MOUNTED_SLOTS });

						pickable_item_to_highlight = typed_interaction.item;

						if (pickup_slot.dead()) {
							pickable_color = bad_color;
						}
					}
				};

				std::visit(handle_interaction, *potential_interaction);
			}

			auto job = [
				&visible,
				&cosm,
				global_time_seconds,
				pickable_item_to_highlight,
				pickable_color,
				shadows  = make_drawing_input(D::DROPPED_ITEMS_SHADOWS),
				borders  = make_drawing_input(D::DROPPED_ITEMS_BORDERS),
				diffuse  = make_drawing_input(D::DROPPED_ITEMS_DIFFUSE),
				neons    = make_drawing_input(D::DROPPED_ITEMS_NEONS),
				overlays = make_drawing_input(D::DROPPED_ITEMS_OVERLAYS)
			]() {
				std::optional<sprite_drawing_input> offset_input;

				visible.for_each<render_layer::ITEMS_ON_GROUND>(
					cosm,
					[&](const auto& handle) {
						handle.template dispatch_on_having_all<invariants::item>([&](const auto& typed_item) {
							const auto is_laying_on_ground = [&]() {
								if (const auto cache = find_colliders_cache(typed_item)) {
									if (cache->constructed_fixtures.size() > 0) {
										if (cache->constructed_fixtures[0]->GetFilterData() == filters[predefined_filter_type::LYING_ITEM]) {
											return true;
										}
									}
								}

								return false;
							}();


							if (!is_laying_on_ground) {
								::specific_draw_entity(typed_item, diffuse);
								::specific_draw_neon_map(typed_item, neons);

								return;
							}

							const auto bounce_dir = vec2(-1, -1);
							const auto bounce_height = 8.f;
							const auto bounce_variation_secs = 2.0 * double(typed_item.get_id().raw.indirection_index);
							const auto bounce_progress = static_cast<float>((1.0 + std::sin(1.5 * (global_time_seconds + bounce_variation_secs) * PI<double>)) / 2.0);

							const auto shadow_alpha = bounce_progress;
							const auto overlay_alpha = 1.0f - bounce_progress;
							const auto bounce_elevation = bounce_progress * bounce_height * bounce_dir;

							auto make_offset_input = [bounce_elevation, &offset_input](const auto& original) -> const auto& {
								offset_input.emplace(original);
								offset_input->renderable_transform += bounce_elevation;
								return *offset_input;
							};

							auto shadow_color = rgba(0, 0, 0, 120);
							shadow_color.mult_alpha(shadow_alpha);

							auto overlay_color = rgba(255, 255, 255, 40);
							overlay_color.mult_alpha(overlay_alpha);

							const bool pickable = typed_item.get_id() == pickable_item_to_highlight;

							::specific_draw_color_highlight(typed_item, shadow_color, shadows);
							::specific_draw_entity(typed_item, diffuse, make_offset_input);
							::specific_draw_neon_map(typed_item, neons, make_offset_input);

							if (!pickable) {
								::specific_draw_color_highlight(typed_item, overlay_color, overlays, make_offset_input);
							}

							if (pickable) {
								auto standard_border_provider = [pickable_color](const auto& typed_handle) -> std::optional<rgba> {
									(void)typed_handle;
									return pickable_color;
								};

								::specific_draw_border(typed_item, borders, standard_border_provider, make_offset_input);
							}
						});
					}
				);
			};

			pool.enqueue(job);
		}

		{
			auto job = [h = make_helper(D::GROUND)]() {
				h.draw<
					render_layer::GROUND,
					render_layer::PLANTED_ITEMS,
					render_layer::OBSTACLES_UNDER_MISSILES,
					render_layer::MISSILES
				>();
			};

			pool.enqueue(job);
		}

		{
			auto job = [h3 = make_helper(D::FOREGROUND_GLOWS)]() {
				h3.draw<
					render_layer::FOREGROUND_GLOWS
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

	auto special_effects_job = [&cosm, &exploding_rings, &dedicated, get_drawer_for, &thunders, queried_cone, actual_cone, get_line_drawer_for]() {
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
				queried_cone,
				actual_cone
			);
		}

		{
			exploding_rings.draw_continuous_rings(
				cosm,
				get_drawer_for(D::EXPLODING_RINGS),
				dedicated[D::EXPLODING_RINGS].specials,
				queried_cone,
				actual_cone
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

