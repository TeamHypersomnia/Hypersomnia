#include <vector>
#include <array>
#include <algorithm>
#include <cmath>

#include "augs/math/rects.h"
#include "augs/drawing/drawing.hpp"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/for_each_entity.h"
#include "game/components/sentience_component.h"
#include "game/components/gun_component.h"
#include "game/components/interpolation_component.h"
#include "game/components/portal_component.h"
#include "game/components/marker_component.h"
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"
#include "game/detail/gun/gun_math.h"
#include "game/detail/crosshair_math.hpp"
#include "game/inferred_caches/physics_world_cache.h"
#include "game/detail/physics/physics_queries.h"
#include "game/enums/filters.h"
#include "game/modes/detail/item_purchase_logic.hpp"
#include "view/audiovisual_state/systems/interpolation_system.h"
#include "view/audiovisual_state/systems/minimap_sighting_system.h"
#include "view/rendering_scripts/calc_laser_path.h"
#include "view/rendering_scripts/draw_minimap.h"
#include "view/rendering_scripts/for_each_iconed_entity.h"

#include "3rdparty/Box2D/Dynamics/b2Fixture.h"
#include "3rdparty/Box2D/Dynamics/b2Body.h"
#include "3rdparty/Box2D/Collision/Shapes/b2PolygonShape.h"
#include "3rdparty/Box2D/Collision/Shapes/b2CircleShape.h"

constexpr float minimap_dot_radius_v = 4.0f;
constexpr int minimap_circle_segments_v = 16;
constexpr float minimap_laser_dash_len_v = 4.0f;
constexpr float minimap_laser_dash_velocity_v = 20.0f;
constexpr float minimap_laser_alpha_mult_v = 0.7f;

void draw_minimap(const draw_minimap_input in) {
	if (in.out_transform != nullptr) {
		in.out_transform->valid = false;
	}

	const auto& settings = in.settings;
	const auto rect = ltrb(calc_minimap_rect(settings, in.screen_size));

	const auto blank = in.blank_tex;
	const auto blank_uv = blank.get_center();

	const auto bg_drawer = augs::drawer { in.solids_output };
	const auto fg_drawer = augs::drawer { in.foreground_output };

	bg_drawer.aabb(blank, rect, settings.background_color);

	auto draw_frame = [&]() {
		auto border = border_input();
		border.width = settings.border_thickness;

		fg_drawer.border(blank, rect, settings.border_color, border);
	};

	const auto viewed = in.viewed_character;

	if (viewed.dead()) {
		draw_frame();
		return;
	}

	const auto viewer_transform = viewed.find_viewing_transform(in.interp);

	if (!viewer_transform.has_value()) {
		draw_frame();
		return;
	}

	auto world_center = viewer_transform->pos;

	const auto fow_size = in.fog_of_war.get_real_size();

	const auto max_fow_side = std::max(fow_size.x, fow_size.y);

	const bool show_entire_map =
		in.extended_range &&
		settings.tab_behavior == minimap_tab_behavior_type::SHOW_ENTIRE_MAP
	;

	auto world_side = max_fow_side * settings.range_mult;

	if (in.extended_range && settings.tab_behavior == minimap_tab_behavior_type::ZOOM_OUT) {
		world_side *= settings.scoreboard_range_mult;
	}

	if (world_side <= 0.0f) {
		draw_frame();
		return;
	}

	const auto& cosm = viewed.get_cosmos();
	const auto& physics = cosm.get_solvable_inferred().physics;
	const auto si = cosm.get_si();

	/*
		If the whole map would fit in a smaller range, zoom in to the
		maximum level at which the entire map still stays visible.
	*/

	{
		auto world_bounds = std::optional<ltrb>();

		/*
			If the player stands on a NAV_ISLAND marker, the island's AABB
			becomes the effective map bounds - maps with several islands
			connected by distant teleports then show only the current one.
		*/

		cosm.for_each_having<invariants::area_marker>(
			[&](const auto& typed_handle) {
				if (world_bounds.has_value()) {
					return;
				}

				const auto& marker = typed_handle.template get<invariants::area_marker>();

				if (marker.type != area_marker_type::NAV_ISLAND) {
					return;
				}

				const auto aabb = typed_handle.find_aabb();

				if (aabb.has_value() && aabb->hover(world_center)) {
					world_bounds = *aabb;
				}
			}
		);

		const bool try_physical_bounds = !world_bounds.has_value();

		const auto obstacle_categories = uint16(
			(1 << int(filter_category::WALL)) |
			(1 << int(filter_category::GLASS_OBSTACLE))
		);

		for (const b2Body* body = try_physical_bounds ? physics.get_b2world().GetBodyList() : nullptr; body != nullptr; body = body->GetNext()) {
			if (body->GetType() != b2_staticBody) {
				continue;
			}

			for (const b2Fixture* f = body->GetFixtureList(); f != nullptr; f = f->GetNext()) {
				if (f->IsSensor()) {
					continue;
				}

				if ((f->GetFilterData().categoryBits & obstacle_categories) == 0) {
					continue;
				}

				const auto* const shape = f->GetShape();

				for (int32 c = 0; c < shape->GetChildCount(); ++c) {
					auto aabb = b2AABB();
					shape->ComputeAABB(&aabb, body->GetTransform(), c);

					const auto fixture_bounds = ltrb::from_points(
						si.get_pixels(vec2(aabb.lowerBound)),
						si.get_pixels(vec2(aabb.upperBound))
					);

					if (world_bounds.has_value()) {
						world_bounds->contain(fixture_bounds);
					}
					else {
						world_bounds = fixture_bounds;
					}
				}
			}
		}

		if (world_bounds.has_value()) {
			/*
				Small maps: some players prefer to always see the whole map
				instead of centering on themselves - as long as the map would
				fit within a reasonable zoom-out.
			*/
			const bool entire_map_fits =
				settings.show_entire_map_if_fits_mult > 0.0f &&
				std::max(world_bounds->w(), world_bounds->h()) <= max_fow_side * settings.show_entire_map_if_fits_mult
			;

			if (show_entire_map || entire_map_fits) {
				/*
					The scoreboard view: the whole map centered,
					regardless of where the player is.
				*/
				world_center = world_bounds->get_center();
				world_side = std::max(world_bounds->w(), world_bounds->h()) * 1.05f;
			}
			else {
				/*
					Position-independent: the worst case is standing in a corner
					of the map and having to see the opposite corner, so twice
					the map's larger dimension always suffices - the zoom stays
					constant no matter where the player is.
				*/
				const auto side_showing_whole_map = 2.0f * std::max(world_bounds->w(), world_bounds->h());

				if (side_showing_whole_map > 0.0f) {
					world_side = std::min(world_side, side_showing_whole_map);
				}
			}
		}
	}

	const auto scale = rect.w() / world_side;
	const auto minimap_center = rect.get_center();

	if (in.out_transform != nullptr) {
		*in.out_transform = { world_center, scale, minimap_center, true };
	}

	auto to_minimap = [&](const vec2 world_pos) {
		return minimap_center + (world_pos - world_center) * scale;
	};

	/*
		Clamps a minimap-space point to the minimap's bounds
		(inset by the drawn element's half size), so that key icons
		out of range stay visible at the border.
	*/
	auto clamp_to_border = [&](const vec2 minimap_pos, const vec2 half_size) {
		if (!settings.clamp_important_to_border) {
			return minimap_pos;
		}

		return vec2(
			std::clamp(minimap_pos.x, rect.l + half_size.x, rect.r - half_size.x),
			std::clamp(minimap_pos.y, rect.t + half_size.y, rect.b - half_size.y)
		);
	};

	auto push_triangle = [&](
		augs::vertex_triangle_buffer& buf,
		const vec2 a,
		const vec2 b,
		const vec2 c,
		const rgba col
	) {
		augs::vertex_triangle tri;

		tri.vertices[0].pos = a;
		tri.vertices[1].pos = b;
		tri.vertices[2].pos = c;

		for (auto& v : tri.vertices) {
			v.color = col;
			v.texcoord = blank_uv;
		}

		buf.push_back(tri);
	};

	auto push_filled_circle = [&](
		augs::vertex_triangle_buffer& buf,
		const vec2 center,
		const float radius,
		const rgba col
	) {
		const auto n = minimap_circle_segments_v;

		for (int i = 0; i < n; ++i) {
			const auto a0 = 360.f * i / n;
			const auto a1 = 360.f * (i + 1) / n;

			push_triangle(
				buf,
				center,
				center + vec2::from_degrees(a0) * radius,
				center + vec2::from_degrees(a1) * radius,
				col
			);
		}
	};

	auto push_circle_ring = [&](
		augs::vertex_triangle_buffer& buf,
		const vec2 center,
		const float inner_r,
		const float outer_r,
		const rgba col
	) {
		const auto n = minimap_circle_segments_v;

		for (int i = 0; i < n; ++i) {
			const auto d0 = vec2::from_degrees(360.f * i / n);
			const auto d1 = vec2::from_degrees(360.f * (i + 1) / n);

			const auto i0 = center + d0 * inner_r;
			const auto i1 = center + d1 * inner_r;
			const auto o0 = center + d0 * outer_r;
			const auto o1 = center + d1 * outer_r;

			push_triangle(buf, i0, o0, o1, col);
			push_triangle(buf, i0, o1, i1, col);
		}
	};

	auto push_line_quad = [&](
		augs::vertex_triangle_buffer& buf,
		const vec2 from,
		const vec2 to,
		const float width,
		const rgba col
	) {
		if ((to - from).length_sq() <= 0.001f) {
			return;
		}

		const auto dir = (to - from).normalize();
		const auto perp = dir.perpendicular_cw() * (width / 2);

		push_triangle(buf, from - perp, to - perp, to + perp, col);
		push_triangle(buf, from - perp, to + perp, from + perp, col);
	};

	auto push_dashed_line = [&](
		augs::vertex_triangle_buffer& buf,
		const vec2 from,
		const vec2 to,
		const rgba col,
		const float dash_length,
		const float dash_velocity
	) {
		const auto line_vector = to - from;
		const auto line_length = line_vector.length();

		if (line_length <= 0.0f) {
			return;
		}

		const auto dir = line_vector / line_length;

		auto dash_end = static_cast<float>(std::fmod(in.global_time_seconds * dash_velocity, dash_length * 2));
		auto dash_begin = std::max(dash_end - dash_length, 0.0f);

		while (dash_begin < line_length) {
			push_line_quad(buf, from + dir * dash_begin, from + dir * dash_end, 1.0f, col);

			dash_begin = dash_end + dash_length;
			dash_end = std::min(dash_begin + dash_length, line_length);
		}
	};

	/*
		Solid obstacles: every fixture participating as WALL or GLASS_OBSTACLE
		within the queried square, drawn straight from the physical shapes.
	*/

	{
		auto obstacles_query = b2Filter();
		obstacles_query.categoryBits = 1 << int(filter_category::QUERY);

		obstacles_query.maskBits =
			(1 << int(filter_category::WALL)) |
			(1 << int(filter_category::GLASS_OBSTACLE))
		;

		const auto query_r = ltrb::center_and_size(world_center, vec2::square(world_side));

		physics.for_each_in_aabb(
			si,
			query_r.left_top(),
			query_r.right_bottom(),
			obstacles_query,
			[&](const b2Fixture& fix) {
				if (fix.IsSensor()) {
					return callback_result::CONTINUE;
				}

				const auto fdata = fix.GetFilterData();

				const bool blocks_bullets = (fdata.maskBits & (1 << int(filter_category::FLYING_BULLET))) != 0;
				const bool blocks_walk = (fdata.maskBits & (1 << int(filter_category::CHARACTER))) != 0;
				const bool see_through = (fdata.categoryBits & (1 << int(filter_category::GLASS_OBSTACLE))) != 0;

				auto col = settings.obstacle_color;

				const auto target_buffer = [&]() -> augs::vertex_triangle_buffer* {
					if (blocks_bullets) {
						if (see_through) {
							/* Glass: same color, half the alpha. */
							col.a /= 2;
						}

						return &in.solids_output;
					}

					if (blocks_walk) {
						/* Walkable for bullets only - dithered. */
						return &in.dither_output;
					}

					return nullptr;
				}();

				if (target_buffer == nullptr) {
					return callback_result::CONTINUE;
				}

				const auto& xf = fix.GetBody()->GetTransform();
				const auto* const shape = fix.GetShape();

				if (shape->GetType() == b2Shape::e_polygon) {
					const auto& poly = static_cast<const b2PolygonShape&>(*shape);
					const auto count = poly.GetVertexCount();

					if (count >= 3) {
						const auto first = to_minimap(si.get_pixels(vec2(b2Mul(xf, poly.GetVertex(0)))));

						auto prev = to_minimap(si.get_pixels(vec2(b2Mul(xf, poly.GetVertex(1)))));

						for (int v = 2; v < count; ++v) {
							const auto next = to_minimap(si.get_pixels(vec2(b2Mul(xf, poly.GetVertex(v)))));

							push_triangle(*target_buffer, first, prev, next, col);
							prev = next;
						}
					}
				}
				else if (shape->GetType() == b2Shape::e_circle) {
					const auto& circle = static_cast<const b2CircleShape&>(*shape);

					const auto center = to_minimap(si.get_pixels(vec2(b2Mul(xf, circle.m_p))));
					const auto radius = si.get_pixels(circle.m_radius) * scale;

					push_filled_circle(*target_buffer, center, radius, col);
				}

				return callback_result::CONTINUE;
			}
		);
	}

	/*
		Portals: circles in their actual size.
	*/

	cosm.for_each_having<components::portal>(
		[&](const auto& typed_handle) {
			using E = remove_cref<decltype(typed_handle)>;

			const auto transform = typed_handle.find_logic_transform();

			if (!transform.has_value()) {
				return;
			}

			const auto& portal = typed_handle.template get<components::portal>();

			if (portal.hide_on_minimap) {
				return;
			}

			const auto color =
				portal.hazard.is_enabled ?
				settings.hazard_color :
				settings.portal_color
			;

			const auto size = typed_handle.get_logical_size();

			const auto shape = [&]() {
				if constexpr(E::template has<components::marker>()) {
					return typed_handle.template get<components::marker>().shape;
				}

				return marker_shape_type::CIRCLE;
			}();

			if (shape == marker_shape_type::BOX) {
				const auto half = size / 2;
				const auto center = transform->pos;
				const auto rotation = transform->rotation;

				const auto corners = std::array<vec2, 4> {
					vec2(-half.x, -half.y),
					vec2(half.x, -half.y),
					vec2(half.x, half.y),
					vec2(-half.x, half.y)
				};

				std::array<vec2, 4> mapped;

				for (std::size_t c = 0; c < corners.size(); ++c) {
					mapped[c] = to_minimap(center + vec2(corners[c]).rotate(rotation));
				}

				push_triangle(in.solids_output, mapped[0], mapped[1], mapped[2], color);
				push_triangle(in.solids_output, mapped[0], mapped[2], mapped[3], color);
			}
			else {
				const auto radius = std::max(size.x, size.y) * 0.5f * scale;

				push_filled_circle(
					in.solids_output,
					to_minimap(transform->pos),
					radius,
					color
				);
			}
		}
	);

	const auto now = cosm.get_total_seconds_passed();
	const auto viewer_faction = viewed.get_official_faction();

	const auto dot_radius = minimap_dot_radius_v * settings.dot_size_mult;

	auto push_pulse = [&](
		const vec2 minimap_pos,
		const double started_at,
		const rgba base_color,
		const float duration_mult,
		const float growth_mult
	) {
		const auto duration = minimap_sighting_system::pulse_duration_secs * duration_mult;
		const auto progress = (now - started_at) / duration;

		if (progress < 0.0 || progress >= 1.0) {
			return;
		}

		const auto t = static_cast<float>(progress);

		auto col = base_color;
		col.mult_alpha(1.0f - t);

		const auto radius = dot_radius * (1.0f + growth_mult * t);

		push_circle_ring(in.foreground_output, minimap_pos, radius - 1.5f, radius + 1.5f, col);
	};

	/*
		Lasers: dashed aiming lines respecting penetration,
		for the viewed character and every conscious teammate.
	*/

	auto draw_laser_of = [&](const auto& character, const bool is_viewer) {
		const auto base_color = [&]() {
			auto col = is_viewer ? settings.player_color : settings.teammate_color;
			col.mult_alpha(minimap_laser_alpha_mult_v);
			return col;
		}();

		for (const auto& item_id : character.get_wielded_items()) {
			const auto item = cosm[item_id];

			if (!item.template has<components::gun>()) {
				continue;
			}

			const auto filter = ::calc_gun_bullet_physical_filter(item);
			const auto gun_transform = item.get_viewing_transform(in.interp);
			const auto muzzle_transform = ::calc_muzzle_transform(item, gun_transform);

			const auto line_from = muzzle_transform.pos;

			const auto line_to = [&]() {
				if (is_viewer) {
					const auto crosshair_pos =
						in.pre_step_crosshair_displacement +
						viewed.get_world_crosshair_transform(in.interp, false).pos
					;

					const auto barrel_center = ::calc_barrel_center(item, gun_transform);
					const auto proj = crosshair_pos.get_projection_multiplier(barrel_center, line_from);

					if (proj > 1.f) {
						return barrel_center + (line_from - barrel_center) * proj;
					}
				}

				return line_from + vec2::from_degrees(muzzle_transform.rotation) * 100;
			}();

			const auto basic_penetration_distance = [&]() {
				if (const auto* const gun_def = item.template find<invariants::gun>()) {
					return gun_def->basic_penetration_distance;
				}

				return 0.0f;
			}();

			thread_local std::vector<laser_path_segment> segments;
			segments.clear();

			::calc_laser_path(
				cosm,
				line_from,
				line_to,
				filter,
				basic_penetration_distance,
				item,
				segments
			);

			const auto dash_velocity = settings.animate_laser_dashes ? minimap_laser_dash_velocity_v : 0.0f;

			for (const auto& seg : segments) {
				push_dashed_line(
					in.foreground_output,
					to_minimap(seg.from),
					to_minimap(seg.to),
					base_color,
					minimap_laser_dash_len_v,
					dash_velocity
				);
			}
		}
	};

	/*
		Bombsite letter markers - drawn under the dots and icons,
		as they only denote terrain.
	*/

	{
		const auto icon_size_mult = 0.75f;

		cosm.for_each_having<invariants::area_marker>(
			[&](const auto& typed_handle) {
				const auto& marker_def = typed_handle.template get<invariants::area_marker>();

				if (marker_def.type != area_marker_type::BOMBSITE) {
					return;
				}

				const auto transform = typed_handle.find_logic_transform();

				if (!transform.has_value()) {
					return;
				}

				const auto& marker = typed_handle.template get<components::marker>();
				const auto tex = in.necessary_images.at(::get_letter_icon(marker.letter));

				const auto center = to_minimap(transform->pos);
				const auto icon_size = vec2(tex.get_original_size()) * icon_size_mult;

				/*
					A darkened square with a dashed border under the letter,
					so it stays legible over the obstacle shapes.
				*/

				const auto bg_rect = ltrb::center_and_size(center, icon_size + vec2::square(8));

				auto bg_col = settings.marker_color;
				bg_col.multiply_rgb(0.2f);
				bg_col.mult_alpha(150.0f / 255);

				auto bg_border_col = settings.marker_color;
				bg_border_col.mult_alpha(200.0f / 255);

				fg_drawer.aabb(blank, bg_rect, bg_col);

				const auto corners = std::array<vec2, 4> {
					bg_rect.left_top(),
					bg_rect.right_top(),
					bg_rect.right_bottom(),
					bg_rect.left_bottom()
				};

				for (std::size_t c = 0; c < corners.size(); ++c) {
					push_dashed_line(
						in.foreground_output,
						corners[c],
						corners[(c + 1) % corners.size()],
						bg_border_col,
						3.0f,
						0.0f
					);
				}

				fg_drawer.aabb_centered(
					tex,
					center,
					icon_size,
					settings.marker_color
				);
			}
		);
	}

	/*
		Tactical icons (death skulls, the bomb) - same sources as the
		offscreen indicators, at 75% of their original size.
	*/

	{
		const auto icon_size_mult = 0.75f;

		for (const auto& special : in.special_indicators) {
			/*
				The minimap-only ones (the carried bomb) are drawn
				later, above the carrier's dot.
			*/
			if (special.minimap_only) {
				continue;
			}

			const auto& tex = special.radar_tex;

			if (!tex.exists()) {
				continue;
			}

			const auto icon_size = vec2(tex.get_original_size()) * icon_size_mult;

			fg_drawer.aabb_centered(
				tex,
				clamp_to_border(to_minimap(special.transform.pos), icon_size / 2),
				icon_size,
				special.color
			);
		}

		/*
			Pulses on the appearing skulls,
			and when the bomb lands on the ground or gets planted.
		*/

		for (const auto& d : in.sighting.recent_deaths) {
			push_pulse(clamp_to_border(to_minimap(d.pos), vec2::square(dot_radius)), d.when, white, 3.0f, 5.0f);
		}

		{
			const auto& bp = in.sighting.bomb_pulse;

			/* The pulse follows the bomb as it slides after being dropped. */
			const auto pulse_pos = [&]() {
				if (const auto bomb = cosm[bp.subject]) {
					const auto transform = bomb.find_viewing_transform(in.interp);

					if (transform.has_value()) {
						return transform->pos;
					}
				}

				return bp.pos;
			}();

			push_pulse(clamp_to_border(to_minimap(pulse_pos), vec2::square(dot_radius)), bp.when, white, 4.0f, 7.0f);
		}
	}

	/*
		The bomb carrier's facing arrow orbits a bit further out,
		so it stays visible next to the bomb icon.
	*/
	auto arrow_radius_of = [&](const entity_id& character_id) {
		return character_id == in.bomb_owner ? dot_radius + 2.0f : dot_radius;
	};

	/*
		A small triangle right at the dot, rotating with the head's facing.
	*/
	auto push_facing_triangle = [&](
		const vec2 minimap_pos,
		const float facing_degrees,
		const float dot_radius,
		const rgba col
	) {
		const auto dir = vec2::from_degrees(facing_degrees);

		const auto tip = minimap_pos + dir * (dot_radius + 6.0f);
		const auto base_center = minimap_pos + dir * (dot_radius + 1.0f);
		const auto side = dir.perpendicular_cw() * 4.5f;

		push_triangle(in.foreground_output, tip, base_center + side, base_center - side, col);
	};

	/*
		Teammates: dots at interpolated positions, plus their lasers.
	*/

	cosm.for_each_having<components::sentience>(
		[&](const auto& typed_handle) {
			const auto& sentience = typed_handle.template get<components::sentience>();

			if (!sentience.is_conscious()) {
				return;
			}

			if (typed_handle.get_official_faction() != viewer_faction) {
				return;
			}

			if (entity_id(typed_handle.get_id()) == entity_id(viewed.get_id())) {
				return;
			}

			const auto transform = typed_handle.find_viewing_transform(in.interp);

			if (!transform.has_value()) {
				return;
			}

			const auto dot_r = dot_radius;
			const auto minimap_pos = clamp_to_border(to_minimap(transform->pos), vec2::square(dot_r));

			push_filled_circle(
				in.foreground_output,
				minimap_pos,
				dot_r,
				settings.teammate_color
			);

			push_facing_triangle(minimap_pos, transform->rotation, arrow_radius_of(entity_id(typed_handle.get_id())), settings.teammate_color);

			draw_laser_of(typed_handle, false);
		}
	);

	/*
		Enemies: dots at the last sighted (or heard) positions,
		with a pulse ring on each (re)appearance.
	*/

	for (const auto& it : in.sighting.enemy_records) {
		const auto& rec = it.second;

		{
			/* The sighting cache might not have caught a faction switch yet. */
			const auto enemy = cosm[it.first];

			if (enemy.alive() && enemy.get_official_faction() == viewer_faction) {
				continue;
			}
		}

		const bool seen_now = in.sighting.is_seen_now(rec, now);
		const bool heard_recently = now - rec.heard_at <= minimap_sighting_system::heard_shot_shows_for_secs;

		auto dot_pos = std::optional<vec2>();
		auto dot_color = settings.enemy_color;
		auto facing = std::optional<float>();
		bool stale = false;

		if (seen_now) {
			if (const auto enemy = cosm[it.first]) {
				const auto transform = enemy.find_viewing_transform(in.interp);

				if (transform.has_value()) {
					dot_pos = transform->pos;
					facing = transform->rotation;
				}
			}
		}
		else if (heard_recently && rec.heard_at > rec.last_seen_at) {
			dot_pos = rec.heard_pos;
		}
		else if (rec.last_seen_at > -1000.0) {
			dot_pos = rec.last_seen_pos;
			dot_color.mult_alpha(0.6f);
			stale = true;
		}

		if (!dot_pos.has_value()) {
			continue;
		}

		/* The last-seen dot is drawn smaller than the live one. */
		auto dot_r = dot_radius;

		if (stale) {
			dot_r = dot_radius * (1.25f / 1.5f);
		}

		const auto minimap_pos = to_minimap(*dot_pos);

		push_filled_circle(
			in.foreground_output,
			minimap_pos,
			dot_r,
			dot_color
		);

		if (facing.has_value()) {
			push_facing_triangle(minimap_pos, *facing, arrow_radius_of(it.first), dot_color);
		}

		push_pulse(minimap_pos, rec.appeared_at, settings.enemy_color, 2.0f, 3.0f);
	}

	/*
		The carried bomb icon - above the dots, but below
		the facing arrow and the viewed player's ring.
	*/

	for (const auto& special : in.special_indicators) {
		if (!special.minimap_only || !special.radar_tex.exists()) {
			continue;
		}

		const auto icon_size = vec2(special.radar_tex.get_original_size()) * 0.75f;

		fg_drawer.aabb_centered(
			special.radar_tex,
			clamp_to_border(to_minimap(special.transform.pos), icon_size / 2),
			icon_size,
			special.color
		);
	}

	/*
		The viewed character: always a dot in the very center.
	*/

	draw_laser_of(viewed, true);

	{
		const auto dot_r = dot_radius;
		const auto minimap_pos = to_minimap(viewer_transform->pos);

		push_filled_circle(
			in.foreground_output,
			minimap_pos,
			dot_r,
			settings.player_color
		);

		push_facing_triangle(minimap_pos, viewer_transform->rotation, arrow_radius_of(viewed.get_id()), settings.player_color);

		/*
			A ring around the viewed character's dot, so it is clear
			who is being watched. The ring opens up where
			the facing arrow points.
		*/

		if (settings.draw_viewed_player_ring) {
			const auto facing = viewer_transform->rotation;

			const auto ring_r = dot_r + 5.0f;
			const auto inner_r = ring_r - 1.5f;
			const auto outer_r = ring_r + 1.5f;

			/*
				The arc is anchored to the facing angle, so its ends
				stay exact while rotating instead of snapping to segments.
			*/

			const auto gap_half_angle = 120.0f;
			const auto arc_begin = facing + gap_half_angle;
			const auto arc_length = 360.0f - 2 * gap_half_angle;
			const auto n = minimap_circle_segments_v * 2;

			for (int i = 0; i < n; ++i) {
				const auto a0 = arc_begin + arc_length * i / n;
				const auto a1 = arc_begin + arc_length * (i + 1) / n;

				const auto d0 = vec2::from_degrees(a0);
				const auto d1 = vec2::from_degrees(a1);

				push_triangle(in.foreground_output, minimap_pos + d0 * inner_r, minimap_pos + d0 * outer_r, minimap_pos + d1 * outer_r, settings.player_color);
				push_triangle(in.foreground_output, minimap_pos + d0 * inner_r, minimap_pos + d1 * outer_r, minimap_pos + d1 * inner_r, settings.player_color);
			}
		}
	}

	draw_frame();
}
