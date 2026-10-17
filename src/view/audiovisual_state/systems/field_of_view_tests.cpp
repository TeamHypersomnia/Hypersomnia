#if !IS_PRODUCTION_BUILD
#if BUILD_UNIT_TESTS
#include <Catch/single_include/catch2/catch.hpp>

#include "augs/math/vec2.h"
#include "augs/math/rects.h"
#include "game/balance_params.h"
#include "game/modes/detail/fog_of_war_settings.h"
#include "game/modes/ai/tasks/ai_field_of_view.h"
#include "view/audiovisual_state/systems/minimap_sighting_system.h"

/*
	The defaults: a 1920x1080 fog of war with a 165 degree angle, and the balance zoom of 0.75.
	A player then sees 2560x1440 on screen, the fog of war spans 5120x2880 around them,
	and the minimap pins within 165 * 0.75 = 123.75 degrees.
*/

using view_type = minimap_sighting_system::observer_view;

static auto default_fow() {
	return fog_of_war_settings();
}

static auto make_viewer_view(
	const float zoom,
	const float edge_zoomout_mult,
	const vec2 screen_center,
	const vec2 screen_size
) {
	const auto expansion = fog_of_war_settings::calc_camera_requested_expansion(zoom, edge_zoomout_mult);

	return view_type {
		default_fow().get_considered(expansion, edge_zoomout_mult),
		ltrb::center_and_size(screen_center, screen_size)
	};
}

static auto at_degrees(const float degrees, const float distance) {
	return vec2::from_degrees(degrees) * distance;
}

TEST_CASE("FieldOfView EffectiveFogOfWar") {
	const auto normal = default_fow().get_considered(fog_of_war_settings::calc_camera_requested_expansion(0.75f, 0.0f), 0.0f);

	REQUIRE(normal.size.x == Approx(2560.0f));
	REQUIRE(normal.size.y == Approx(1440.0f));
	REQUIRE(normal.angle == Approx(165.0f));

	const auto edge = default_fow().get_considered(fog_of_war_settings::calc_camera_requested_expansion(0.75f, 1.0f), 1.0f);

	REQUIRE(edge.size.x == Approx(2560.0f / BALANCE_MAX_ZOOM_OUT_AT_EDGES));
	REQUIRE(edge.angle == Approx(165.0f * BALANCE_EDGE_ZOOM_OUT_FOW_ANGLE_MULT));

	const auto half_way = default_fow().get_considered(1.0f, 0.5f);
	REQUIRE(half_way.angle == Approx(165.0f * (1.0f + BALANCE_EDGE_ZOOM_OUT_FOW_ANGLE_MULT) / 2));

	SECTION("A disabled fog of war stays disabled at the screen edge") {
		for (const auto disabled_angle : { 0.0f, 360.0f }) {
			auto fow = default_fow();
			fow.angle = disabled_angle;

			REQUIRE_FALSE(fow.get_considered(1.0f, 1.0f).is_enabled());
			REQUIRE_FALSE(fow.get_considered(1.0f, 0.5f).is_enabled());
		}
	}
}

TEST_CASE("FieldOfView MinimapViewer") {
	const auto origin = vec2::zero;
	const auto right = vec2(1, 0);

	SECTION("In front and on screen") {
		const auto view = make_viewer_view(0.75f, 0.0f, origin, vec2(2560, 1440));
		REQUIRE(view.shows(origin, right, vec2(1000, 0)));
	}

	SECTION("Behind") {
		const auto view = make_viewer_view(0.75f, 0.0f, origin, vec2(2560, 1440));
		REQUIRE_FALSE(view.shows(origin, right, vec2(-1000, 0)));
	}

	SECTION("In the fog of war but off the screen shifted by the crosshair") {
		/* The camera looks 500 px to the right: the screen spans x from -780 to 1780. */
		const auto view = make_viewer_view(0.75f, 0.0f, vec2(500, 0), vec2(2560, 1440));

		REQUIRE(view.shows(origin, right, vec2(1700, 0)));
		REQUIRE_FALSE(view.shows(origin, right, vec2(1900, 0)));
	}

	SECTION("Within the view angle but outside the narrower pinning cone") {
		const auto view = make_viewer_view(0.75f, 0.0f, origin, vec2(2560, 1440));
		const auto pin_half_angle = 165.0f * BALANCE_MINIMAP_SIGHTING_FOV_ANGLE_MULT / 2;

		REQUIRE(pin_half_angle < 165.0f / 2 - 5.0f);
		REQUIRE(view.shows(origin, right, at_degrees(pin_half_angle - 5.0f, 600.0f)));
		REQUIRE(view.shows(origin, right, at_degrees(-(pin_half_angle - 5.0f), 600.0f)));
		REQUIRE_FALSE(view.shows(origin, right, at_degrees(pin_half_angle + 5.0f, 600.0f)));
		REQUIRE_FALSE(view.shows(origin, right, at_degrees(-(pin_half_angle + 5.0f), 600.0f)));
	}

	SECTION("The cone follows the look direction") {
		const auto view = make_viewer_view(0.75f, 0.0f, origin, vec2(2560, 1440));
		const auto up = vec2(0, -1);

		REQUIRE(view.shows(origin, up, vec2(0, -500)));
		REQUIRE_FALSE(view.shows(origin, up, vec2(600, 0)));
	}

	SECTION("Edge zoom-out sees farther within a much narrower angle") {
		/*
			At full edge zoom-out the angle narrows to 165 * BALANCE_EDGE_ZOOM_OUT_FOW_ANGLE_MULT,
			and the pinning cone is BALANCE_MINIMAP_SIGHTING_FOV_ANGLE_MULT of that.
		*/

		const auto view = make_viewer_view(0.75f, 1.0f, vec2(1500, 0), vec2(3657, 2057));
		const auto pin_half_angle = 165.0f * BALANCE_EDGE_ZOOM_OUT_FOW_ANGLE_MULT * BALANCE_MINIMAP_SIGHTING_FOV_ANGLE_MULT / 2;

		REQUIRE(view.shows(origin, right, vec2(3000, 0)));
		REQUIRE(view.shows(origin, right, at_degrees(pin_half_angle - 1.0f, 3000.0f)));
		REQUIRE_FALSE(view.shows(origin, right, at_degrees(pin_half_angle + 1.0f, 3000.0f)));

		/* Beyond the normal fog of war (2560), within the expanded one (3657). */
		const auto normal_view = make_viewer_view(0.75f, 0.0f, vec2(1500, 0), vec2(3657, 2057));
		REQUIRE_FALSE(normal_view.shows(origin, right, vec2(3000, 0)));
	}

	SECTION("A wider screen than the fog of war still ends at the fog of war") {
		const auto view = make_viewer_view(0.75f, 0.0f, origin, vec2(6000, 1440));

		REQUIRE(view.shows(origin, right, vec2(2500, 0)));
		REQUIRE_FALSE(view.shows(origin, right, vec2(2700, 0)));
	}

	SECTION("A disabled fog of war has no angle limit") {
		auto view = make_viewer_view(0.75f, 0.0f, origin, vec2(2560, 1440));
		view.fow.angle = 360.0f;

		REQUIRE(view.shows(origin, right, vec2(-500, 0)));
		REQUIRE_FALSE(view.shows(origin, right, vec2(-1500, 0)));
	}

	SECTION("A disabled fog of war has no angle limit at the screen edge either") {
		auto fow = default_fow();
		fow.angle = 360.0f;

		const auto view = view_type {
			fow.get_considered(fog_of_war_settings::calc_camera_requested_expansion(0.75f, 1.0f), 1.0f),
			ltrb::center_and_size(origin, vec2(2560, 1440))
		};

		REQUIRE(view.shows(origin, right, vec2(-500, 0)));
	}
}

TEST_CASE("FieldOfView MinimapTeammate") {
	const auto right = vec2(1, 0);

	SECTION("The screen follows the teammate's crosshair") {
		const auto teammate_pos = vec2(100, 100);
		const auto view = minimap_sighting_system::calc_teammate_view(default_fow(), 0.75f, teammate_pos, vec2(400, 0));

		REQUIRE(view.fow.size.x == Approx(2560.0f));
		REQUIRE(view.screen_area.l == Approx(500.0f - 1280.0f));
		REQUIRE(view.screen_area.r == Approx(500.0f + 1280.0f));
		REQUIRE(view.screen_area.t == Approx(100.0f - 720.0f));
		REQUIRE(view.screen_area.b == Approx(100.0f + 720.0f));

		REQUIRE(view.shows(teammate_pos, right, vec2(1700, 100)));
		REQUIRE_FALSE(view.shows(teammate_pos, right, vec2(1850, 100)));
		REQUIRE_FALSE(view.shows(teammate_pos, right, vec2(-500, 100)));
	}

	SECTION("The screen moves at most as far as a player's crosshair could move it") {
		/* A bot aims right at a target far beyond the screen edge: the camera moves by at most half the screen. */
		const auto view = minimap_sighting_system::calc_teammate_view(default_fow(), 0.75f, vec2::zero, vec2(2500, -2500));

		REQUIRE(view.screen_area.l == Approx(0.0f).margin(0.01f));
		REQUIRE(view.screen_area.r == Approx(2560.0f));
		REQUIRE(view.screen_area.t == Approx(-1440.0f));
		REQUIRE(view.screen_area.b == Approx(0.0f).margin(0.01f));
	}

	SECTION("A zoomed in camera zoom area shows less") {
		const auto view = minimap_sighting_system::calc_teammate_view(default_fow(), 1.5f, vec2::zero, vec2::zero);

		REQUIRE(view.fow.size.x == Approx(1280.0f));
		REQUIRE(view.shows(vec2::zero, right, vec2(600, 0)));
		REQUIRE_FALSE(view.shows(vec2::zero, right, vec2(700, 0)));
	}

	SECTION("The teammate pins within the same narrower cone") {
		const auto view = minimap_sighting_system::calc_teammate_view(default_fow(), 0.75f, vec2::zero, vec2::zero);

		const auto pin_half_angle = 165.0f * BALANCE_MINIMAP_SIGHTING_FOV_ANGLE_MULT / 2;

		REQUIRE(view.shows(vec2::zero, right, at_degrees(pin_half_angle - 5.0f, 600.0f)));
		REQUIRE_FALSE(view.shows(vec2::zero, right, at_degrees(pin_half_angle + 5.0f, 600.0f)));
	}
}

TEST_CASE("FieldOfView Bots") {
	const auto origin = vec2::zero;
	const auto right = vec2(1, 0);

	SECTION("Difficulty multipliers") {
		REQUIRE(::get_bot_fov_mult(difficulty_type::VERY_EASY) < ::get_bot_fov_mult(difficulty_type::EASY));
		REQUIRE(::get_bot_fov_mult(difficulty_type::EASY) < ::get_bot_fov_mult(difficulty_type::MEDIUM));
		REQUIRE(::get_bot_fov_mult(difficulty_type::MEDIUM) < ::get_bot_fov_mult(difficulty_type::HARD));
		REQUIRE(::get_bot_fov_mult(difficulty_type::HARD) <= 1.0f);
		REQUIRE(::get_bot_fov_mult(difficulty_type::PROGRESSIVE) == ::get_bot_fov_mult(difficulty_type::MEDIUM));
	}

	SECTION("Proportional to the player's view") {
		const auto mult = 0.9f;
		const auto fov = ::calc_ai_field_of_view(default_fow(), 0.75f, mult, false);

		REQUIRE(fov.half_size.x == Approx(2560.0f * mult));
		REQUIRE(fov.half_size.y == Approx(1440.0f * mult));
		REQUIRE(fov.angle == Approx(165.0f * mult));

		REQUIRE(fov.within_box(origin, vec2(2560.0f * mult - 10, 0)));
		REQUIRE_FALSE(fov.within_box(origin, vec2(2560.0f * mult + 10, 0)));
		REQUIRE_FALSE(fov.within_box(origin, vec2(0, 1440.0f * mult + 10)));
		REQUIRE_FALSE(fov.within_box(origin, vec2(-2560.0f * mult - 10, 0)));

		const auto half_angle = 165.0f * mult / 2;

		REQUIRE(fov.within_angle(right, vec2::from_degrees(half_angle - 1)));
		REQUIRE(fov.within_angle(right, vec2::from_degrees(-(half_angle - 1))));
		REQUIRE_FALSE(fov.within_angle(right, vec2::from_degrees(half_angle + 1)));
		REQUIRE_FALSE(fov.within_angle(right, vec2(-1, 0)));
	}

	SECTION("The camera zoom scales the range, not the angle") {
		const auto zoomed_in = ::calc_ai_field_of_view(default_fow(), 1.5f, 1.0f, false);
		const auto zoomed_out = ::calc_ai_field_of_view(default_fow(), 0.5f, 1.0f, false);

		REQUIRE(zoomed_in.half_size.x == Approx(1280.0f));
		REQUIRE(zoomed_out.half_size.x == Approx(3840.0f));
		REQUIRE(zoomed_in.angle == Approx(165.0f));
		REQUIRE(zoomed_out.angle == Approx(165.0f));
	}

	SECTION("Looking far is like the player's edge zoom-out") {
		const auto mult = 0.9f;
		const auto fov = ::calc_ai_field_of_view(default_fow(), 0.75f, mult, true);
		const auto player = default_fow().get_considered(fog_of_war_settings::calc_camera_requested_expansion(0.75f, 1.0f), 1.0f);

		REQUIRE(fov.half_size.x == Approx(player.size.x * mult));
		REQUIRE(fov.half_size.y == Approx(player.size.y * mult));
		REQUIRE(fov.angle == Approx(player.angle * mult));

		const auto half_angle = player.angle * mult / 2;

		REQUIRE(fov.within_box(origin, vec2(3000, 0)));
		REQUIRE(fov.within_angle(right, vec2::from_degrees(half_angle - 1.0f)));
		REQUIRE_FALSE(fov.within_angle(right, vec2::from_degrees(half_angle + 1.0f)));
	}

	SECTION("A disabled fog of war has no angle limit") {
		auto fow = default_fow();
		fow.angle = 360.0f;

		for (const auto disabled_angle : { 0.0f, 360.0f }) {
			fow.angle = disabled_angle;

			for (const bool looks_far : { false, true }) {
				const auto fov = ::calc_ai_field_of_view(fow, 0.75f, 0.9f, looks_far);

				REQUIRE_FALSE(fov.considers_angle());
				REQUIRE(fov.within_angle(right, vec2(-1, 0)));
			}
		}
	}
}
#endif
#endif
