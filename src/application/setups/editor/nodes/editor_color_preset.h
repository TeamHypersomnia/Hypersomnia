#pragma once

enum class editor_color_preset {
	// GEN INTROSPECTOR enum class editor_color_preset
	CUSTOM,
	CYAN,
	ORANGE,
	RED,
	GREEN
	// END GEN INTROSPECTOR
};

struct editor_color_preset_info {
	struct portal_info {
		rgba inner_ring;
		rgba outer_ring;
		rgba light;
		rgba particle_ambience;
	};

	portal_info portal;
};

inline auto get_editor_color_presets() {
	auto result = std::unordered_map<editor_color_preset, editor_color_preset_info>();

	auto& cy = result[editor_color_preset::CYAN];
	cy.portal.inner_ring = cyan;
	cy.portal.outer_ring = turquoise.with_alpha(200);
	cy.portal.light = cyan.with_alpha(150);
	cy.portal.particle_ambience = cyan;

	auto& oe = result[editor_color_preset::ORANGE];
	oe.portal.inner_ring = yellow;
	oe.portal.outer_ring = rgba(222, 167, 0, 200);
	oe.portal.light = rgba(222, 167, 0, 200);
	oe.portal.particle_ambience = rgba(222, 167, 0, 255);

	auto& gr = result[editor_color_preset::GREEN];
	gr.portal.inner_ring = green;
	gr.portal.outer_ring = green.with_alpha(200);
	gr.portal.light = green.with_alpha(150);
	gr.portal.particle_ambience = green;

	auto& rd = result[editor_color_preset::RED];
	rd.portal.inner_ring = red;
	rd.portal.outer_ring = rgba(128, 0, 0, 200);
	rd.portal.light = red.with_alpha(150);
	rd.portal.particle_ambience = red;

	return result;
}

