#pragma once
#include <string>
#include <vector>
#include <optional>

#include "augs/math/vec2.h"
#include "augs/graphics/rgba.h"
#include "augs/misc/randomization.h"
#include "view/necessary_image_id.h"

/*
	The common look of the custom HUD progress bars:
	the character's value bars, the money bar,
	the tutorial's bottom progress bar and its stage bar.
	Every bar sets its own hud_bar_appearance at the call site.
*/

/* A bar is never divided into more segments than this. */
constexpr int max_hud_bar_splits_v = 64;

struct hud_bar_appearance {
	rgba color = white;
	int border_w = 1;
	float particle_tint = 0.0f;

	/*
		When splits > 1, the bar is divided into that many segments,
		each with its own borders - as if several bars were drawn side by side.
		With split_gap == 0, the neighboring segments share
		a single 1px outer black outline between them;
		with split_gap >= 1, each draws its own, that many pixels apart.
	*/
	int splits = 0;
	int split_gap = 0;

	/*
		Whether the gains keep up to 16 independent, concurrently fading
		flashes (with continuous growth merging into one) - or restart
		a single flash on every gain.
	*/
	bool multiple_flashes = false;

	/*
		Once the bar gets full, flash its whole length
		instead of only the last gain.
	*/
	bool flash_whole_on_full = false;

	/*
		Optional text over the bar, at full opacity,
		stroked black, in the particle color.
		The caller formats the value into the string itself.
	*/
	std::string label;

	/*
		Whether to draw a backdrop under the label - the text's bounding box
		filled with the bar's background shade, framed like the bar itself:
		a border in the bar's bright color plus black outlines around it.
		The border doubles as a value indicator: it is scissored
		horizontally at the fill's position, and flowing particles
		wander inside its filled part.
		label_padding is the total padding added to the text's bounding box.
	*/
	bool label_background = false;
	vec2i label_padding = vec2i(10, 4);

	/* Aligns the backdrop's bottom exactly with the bar's bottom edge. */
	bool label_align_bottom = false;

	/*
		Anchors the label block to the bar's right edge, with a constant
		width fitting label_widest_text - so the labels of stacked
		bars line up in a column. The bar ends where the label begins.
	*/
	bool label_align_right = false;
	std::string label_widest_text;

	/* The width of the backdrop's luminous border. */
	int label_border_w = 2;

	/*
		The unfilled part of the luminous border is still drawn,
		just this thin - hugging the band's outer edge - so the brick's
		silhouette always reads whole, and the filled part looks thickened.
	*/
	int label_unfilled_border_w = 1;

	/*
		Overrides the backdrop's shade (the bar color darkened by default),
		e.g. to darken it further for the text to read better.
	*/
	std::optional<rgba> label_background_color;
};

struct hud_bar_particle {
	vec2i relative_pos;
	assets::necessary_image_id image_id = assets::necessary_image_id::INVALID;
};

struct hud_bar_particles_state {
	std::vector<hud_bar_particle> particles;
	double last_advance_secs = 0.0;
	vec2i spawned_for_size;

	/* Seeded anew on every spawn, so that the bars do not wander in lockstep. */
	randomization rng;
};

struct hud_bar_flash {
	float from_ratio = 0.0f;
	float to_ratio = 0.0f;
	double at_secs = -1.0;
};

/*
	Detects the bar's growth at draw time,
	to know when (and over what part) to draw the white flash.
*/

struct hud_bar_highlight_state {
	float last_ratio = -1.0f;
	double last_update_secs = 0.0;

	/*
		Every gain spawns its own flash, so the earlier ones
		keep fading out undisturbed when new gains arrive.
	*/
	std::vector<hud_bar_flash> flashes;
};

/* Draw-time caches of a single bar. */

struct hud_bar_state {
	hud_bar_highlight_state highlight;
	hud_bar_particles_state particles;
	hud_bar_particles_state label_particles;
};
