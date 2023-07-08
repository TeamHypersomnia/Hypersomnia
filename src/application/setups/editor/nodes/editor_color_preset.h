#pragma once

enum class editor_color_preset {
	// GEN INTROSPECTOR enum class editor_color_preset
	CUSTOM,
	CYAN,
	YELLOW,
	RED,
	GREEN,
	PINK,
	LAVA,
	WHITE,
	COUNT
	// END GEN INTROSPECTOR
};

struct editor_color_preset_info {
	struct portal_info {
		rgba inner_ring = white;
		rgba outer_ring = white;
		rgba light = white;
		rgba ambience_particles = white;
		rgba begin_entering_particles = white;
		rgba enter_particles = white;
		rgba exit_particles = white;
	};

	portal_info portal;
};

inline auto get_editor_color_preset(const editor_color_preset t) {
	auto o = editor_color_preset_info();

	switch (t) {
		case editor_color_preset::CYAN:
			o.portal.inner_ring = cyan;
			o.portal.outer_ring = turquoise.with_alpha(200);
			o.portal.light = cyan.with_alpha(150);
			o.portal.ambience_particles = cyan;
			o.portal.begin_entering_particles = o.portal.enter_particles = o.portal.exit_particles = o.portal.ambience_particles;
			break;

		case editor_color_preset::YELLOW:
			o.portal.inner_ring = yellow;
			o.portal.outer_ring = rgba(222, 167, 0, 200);
			o.portal.light = rgba(222, 167, 0, 200);
			o.portal.ambience_particles = rgba(222, 167, 0, 255);
			o.portal.begin_entering_particles = o.portal.enter_particles = o.portal.exit_particles = o.portal.ambience_particles;
			break;

		case editor_color_preset::GREEN:
			o.portal.inner_ring = green;
			o.portal.outer_ring = green.with_alpha(200);
			o.portal.light = green.with_alpha(150);
			o.portal.ambience_particles = green;
			o.portal.begin_entering_particles = o.portal.enter_particles = o.portal.exit_particles = o.portal.ambience_particles;
			break;

		case editor_color_preset::RED:
			o.portal.inner_ring = red;
			o.portal.outer_ring = rgba(128, 0, 0, 200);
			o.portal.light = red.with_alpha(150);
			o.portal.ambience_particles = red;
			o.portal.begin_entering_particles = o.portal.enter_particles = o.portal.exit_particles = o.portal.ambience_particles;
			break;

		case editor_color_preset::LAVA:
			o.portal.inner_ring = rgba(255, 102, 0, 255);
			o.portal.outer_ring = rgba(128, 0, 0, 200);
			o.portal.light = red.with_alpha(150);
			o.portal.ambience_particles = rgba(255, 102, 0, 255);
			o.portal.begin_entering_particles = o.portal.enter_particles = o.portal.exit_particles = o.portal.ambience_particles;
			break;

		case editor_color_preset::PINK:
			o.portal.inner_ring = pink;
			o.portal.outer_ring = rgba(143, 0, 255, 200);
			o.portal.light = rgba(143, 0, 255, 150);
			o.portal.ambience_particles = pink;
			o.portal.begin_entering_particles = o.portal.enter_particles = o.portal.exit_particles = o.portal.ambience_particles;
			break;

		default:
			break;
	}

	return o;
}

