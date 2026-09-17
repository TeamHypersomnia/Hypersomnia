#pragma once
#include <cstddef>
#include <cmath>
#include <array>
#include <string>
#include <vector>
#include <optional>
#include <algorithm>

#include "augs/math/vec2.h"
#include "augs/math/rects.h"
#include "augs/math/arithmetical.h"
#include "augs/graphics/rgba.h"
#include "augs/drawing/drawing.h"
#include "augs/misc/randomization.h"
#include "augs/gui/text/printer.h"
#include "view/necessary_image_id.h"
#include "view/necessary_resources.h"

/*
	The common look of the custom HUD progress bars:
	the money bar, the tutorial's bottom progress bar and its stage bar.
	Every bar sets its own hud_bar_appearance at the call site.
*/

/*
	Whether the white flash shown when a bar's value increases
	should also tint the bar's border, not just the inner fill.
*/
constexpr bool bar_increase_highlight_affects_border_v = false;

/*
	Whether the flash covers only the newly added part of the bar,
	instead of the whole filled part.
*/
constexpr bool bar_increase_highlight_only_added_part_v = true;

/*
	The flash lasts this much longer than the white highlight
	of a damaged character's health bar.
*/
constexpr float bar_increase_highlight_duration_mult_v = 2.0f;

/* A bar is never divided into more segments than this. */
constexpr int max_hud_bar_splits_v = 20;

struct hud_bar_appearance {
	rgba color = white;
	int border_w = 1;
	bool black_frame = true;
	bool dark_background = true;
	bool fill_edge_outline = true;
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
		Renders the two middle segments as one, so that no division
		hides under the centered label - the value still fills
		by the original equal slices.
	*/
	bool trisplit = false;

	/*
		Optional text centered over the whole bar, at full opacity,
		stroked black - in the particle color unless label_color is set.
		The caller formats the value into the string itself.
	*/
	std::string label;
	std::optional<rgba> label_color;

	/*
		Whether to draw a backdrop under the label - the text's bounding box
		filled with the bar's background shade, framed like the bar itself:
		a border in the bar's bright color plus black outlines around it.
		label_padding is the total padding added to the text's bounding box.
	*/
	bool label_background = false;
	vec2i label_padding = vec2i(10, 4);

	/* Aligns the backdrop's bottom exactly with the bar's bottom edge. */
	bool label_align_bottom = false;

	/*
		Anchors the label block to the bar's right edge, with a constant
		backdrop width fitting label_widest_text - so the labels of stacked
		bars line up in a column. The text itself is right-aligned too.
	*/
	bool label_align_right = false;
	std::string label_widest_text;

	/* The width of the backdrop's luminous border. */
	int label_border_w = 2;

	/*
		The luminous border doubles as a value indicator: it is scissored
		horizontally at the covered bar's fill edge - down to single pixel
		columns of the side edges' thickness. Where it is cut away,
		the backdrop's background shade shows in its place.
		The dark 1px outline inside the border can sweep along with it.
		With full_range_sweep, the value range maps linearly onto the band's
		whole width instead of following the bar's actual fill edge.
	*/
	bool label_border_shows_value = true;
	bool label_inner_outline_shows_value = true;
	bool label_border_full_range_sweep = false;

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

	/*
		When set, the label block is lifted so that this many pixels
		of the bar's actual interior (not its borders) stay visible
		below the backdrop's frame - with large values the text
		may stick out entirely above the bar.
	*/
	std::optional<int> label_reveal_bottom_px;
};

struct hud_bar_particle {
	vec2i relative_pos;
	assets::necessary_image_id image_id = assets::necessary_image_id::INVALID;
};

struct hud_bar_particles_state {
	std::vector<hud_bar_particle> particles;
	double last_advance_secs = 0.0;
	vec2i spawned_for_size;
};

/*
	Detects the bar's growth at draw time,
	to know when (and over what part) to draw the white flash.
*/

struct hud_bar_highlight_state {
	float last_ratio = -1.0f;
	float from_ratio = 0.0f;
	float to_ratio = 0.0f;
	double at_secs = -1.0;
	double last_update_secs = 0.0;
};

inline void update_hud_bar_highlight(
	hud_bar_highlight_state& highlight,
	const float ratio,
	const double total_secs
) {
	if (total_secs < highlight.last_update_secs) {
		/*
			The cosmos' clock went back - a round restart or a demo seek.
			Start tracking afresh: the jump back to the initial values
			must not read as a gain, and a pending flash whose timestamp
			comes from the previous timeline must not replay.
		*/

		highlight.at_secs = -1.0;
		highlight.last_ratio = ratio;
	}

	highlight.last_update_secs = total_secs;

	if (highlight.last_ratio >= 0.0f && ratio > highlight.last_ratio) {
		highlight.at_secs = total_secs;
		highlight.from_ratio = highlight.last_ratio;
		highlight.to_ratio = ratio;
	}

	highlight.last_ratio = ratio;
}

/*
	The same flowing particles the character's value bars draw
	inside their filled parts. The particles live in a virtual field
	spanning the concatenation of all the segments' interiors,
	so they flow continuously across the split borders.
*/

inline void advance_hud_bar_particles(
	hud_bar_particles_state& state,
	const double total_secs,
	const vec2i field_size
) {
	if (state.particles.empty() || state.spawned_for_size != field_size) {
		state.particles.clear();
		state.spawned_for_size = field_size;

		thread_local randomization rng;

		/* The same density the character's value bars use: 40 particles per a 160px bar. */
		const auto num_particles_to_spawn = static_cast<std::size_t>(std::max(1, field_size.x / 4));

		for (std::size_t i = 0; i < num_particles_to_spawn; ++i) {
			const auto mats = std::array<assets::necessary_image_id, 3> {
				assets::necessary_image_id::WANDERING_CROSS,
				assets::necessary_image_id::BLINK_1,
				static_cast<assets::necessary_image_id>(static_cast<int>(assets::necessary_image_id::BLINK_1) + 2),
			};

			auto new_part = hud_bar_particle();
			new_part.relative_pos = rng.randval(vec2(0, 0), vec2(field_size));
			new_part.image_id = rng.choose_from(mats);

			state.particles.push_back(new_part);
		}
	}

	if (total_secs < state.last_advance_secs) {
		/* The cosmos' time went back, e.g. after a restart. */
		state.last_advance_secs = total_secs;
	}

	/* The same 15 Hz wandering the character's value bars use. */

	randomization rng{ static_cast<rng_seed_type>(total_secs * 1000) };

	while (total_secs - state.last_advance_secs > 1.0 / 15) {
		for (auto& p : state.particles) {
			if (rng.randval(0, 8) == 0) {
				p.relative_pos.y += rng.randval(0, 1) == 0 ? 1 : -1;
			}

			++p.relative_pos.x;

			p.relative_pos.y = std::max(0, p.relative_pos.y);
			p.relative_pos.x = std::max(0, p.relative_pos.x);

			p.relative_pos.x %= field_size.x + 12;
			p.relative_pos.y %= field_size.y + 12;
		}

		state.last_advance_secs += 1.0 / 15;
	}
}

inline void draw_hud_bar_particles(
	const hud_bar_particles_state& state,
	const augs::drawer_with_default output,
	const necessary_images_in_atlas_map& necessary_images,
	const vec2 field_origin,
	const ltrb clipper,
	const rgba particle_col
) {
	if (clipper.w() < 1) {
		return;
	}

	for (const auto& p : state.particles) {
		output.aabb_lt_clipped(
			necessary_images.at(p.image_id),
			field_origin - vec2(6, 6) + vec2(p.relative_pos),
			clipper,
			particle_col
		);
	}
}

/*
	Draws a complete bar: the black frame, the darkened background,
	the fill with its bright border, the outline at the fill's moving edge,
	the flowing particles, the white flash on increase
	and the optional centered label.

	bordered_rect is the outer rect of the bright border.
	Pass nullptr for particles to draw a bar without them,
	and a font to draw the appearance's label.
*/

inline void draw_hud_bar(
	const augs::drawer_with_default output,
	const necessary_images_in_atlas_map& necessary_images,
	const hud_bar_appearance& appearance,
	const ltrb bordered_rect,
	const float ratio,
	const double total_secs,
	const float highlight_base_duration_secs,
	hud_bar_highlight_state& highlight,
	hud_bar_particles_state* const particles,
	const augs::baked_font* const label_font = nullptr
) {
	const auto bar_col = appearance.color;
	const auto clamped_ratio = std::clamp(ratio, 0.0f, 1.0f);

	::update_hud_bar_highlight(highlight, clamped_ratio, total_secs);

	const auto bar_border = border_input { appearance.border_w, 1 };
	const auto border_expansion = bar_border.get_total_expansion();

	auto dark_bar_col = rgba(bar_col) * 0.4f;
	dark_bar_col.a = 200;

	auto particle_col = augs::interp(white, bar_col, appearance.particle_tint);
	particle_col.a = 220;

	/*
		A right-aligned label brick truncates the bar: since the brick's
		backdrop is translucent, nothing may render underneath it -
		the bar (its frames included) ends right where the brick begins.
		This is the x the bar's outermost right edge may reach.
	*/

	const auto bar_truncate_r = [&]() -> std::optional<float> {
		if (label_font == nullptr || appearance.label.empty() || !appearance.label_background || !appearance.label_align_right) {
			return std::nullopt;
		}

		using namespace augs::gui::text;

		const auto label_bw = std::max(1, appearance.label_border_w);
		const auto frame_expansion = 1 + label_bw + 1;
		const auto pad_x = appearance.label_padding.x - 2 * label_bw;

		const auto& measured_text = appearance.label_widest_text.empty() ? appearance.label : appearance.label_widest_text;
		const auto text_w = get_text_bbox(formatted_string(measured_text, style(*label_font, white))).x;

		const auto bar_outer_r = static_cast<int>(bordered_rect.r) + (appearance.black_frame ? 1 : 0);
		const auto backdrop_l = bar_outer_r - frame_expansion - (text_w + pad_x);
		const auto brick_outer_l = backdrop_l - frame_expansion;

		return static_cast<float>(brick_outer_l);
	}();

	/*
		The segment layout. A single bar is just one segment spanning
		the whole bordered_rect. The division leftover widens the first segment.
	*/

	struct bar_segment {
		ltrb inner;
		float fill_w = 0.0f;

		/* Where the segment's interior begins in the concatenated inner space. */
		int inner_offset = 0;

		/* The value slices this segment covers (a merged one covers two). */
		int slice_index = 0;
		int slice_count = 1;

		/* Interior pixels beyond visible_r hide under the label brick. */
		float visible_r = 0.0f;

		/* The right frame is not drawn - the brick terminates the segment. */
		bool open_right = false;
	};

	std::array<bar_segment, max_hud_bar_splits_v> segments;

	const auto num_slices = std::clamp(std::max(appearance.splits, 1), 1, max_hud_bar_splits_v);
	const auto separation = num_slices > 1 ? (appearance.split_gap == 0 ? 1 : appearance.split_gap + 2) : 0;

	const auto total_t = bordered_rect.t;
	const auto total_b = bordered_rect.b;

	{
		const auto total_w = static_cast<int>(bordered_rect.w());
		const auto segments_w = total_w - (num_slices - 1) * separation;

		const auto base_w = segments_w / num_slices;
		const auto leftover_w = segments_w % num_slices;

		auto x = static_cast<int>(bordered_rect.l);

		for (int i = 0; i < num_slices; ++i) {
			/* The division leftover spreads one pixel per segment, first ones first. */
			const auto this_w = base_w + (i < leftover_w ? 1 : 0);

			auto& seg = segments[i];
			seg.inner = ltrb(ltrb(x, total_t, x + this_w, total_b)).expand_from_center(vec2::square(static_cast<float>(-border_expansion)));
			seg.slice_index = i;
			seg.slice_count = 1;

			x += this_w + separation;
		}
	}

	auto num_segments = num_slices;

	/*
		With trisplit, the two middle segments render as one -
		no pointless division under the centered label - while the value
		semantics stay in the original equal slices.
	*/

	if (appearance.trisplit && num_slices >= 4 && num_slices % 2 == 0) {
		const auto mid = num_slices / 2 - 1;

		segments[mid].inner.r = segments[mid + 1].inner.r;
		segments[mid].slice_count = 2;

		for (int i = mid + 1; i + 1 < num_slices; ++i) {
			segments[i] = segments[i + 1];
		}

		num_segments = num_slices - 1;
	}

	for (int i = 0; i < num_segments; ++i) {
		segments[i].visible_r = segments[i].inner.r;
	}

	/*
		The brick covers the bar's tail: the crossed segment stays OPEN
		on its right - it runs up to and touches the brick, which terminates
		it in place of its right frame.
	*/

	if (bar_truncate_r.has_value()) {
		const auto touch_x = *bar_truncate_r;

		for (int i = 0; i < num_segments; ++i) {
			auto& seg = segments[i];

			const auto outer_r = seg.inner.r + border_expansion + (appearance.black_frame ? 1 : 0);

			if (outer_r > touch_x) {
				seg.open_right = true;
				seg.visible_r = std::clamp(touch_x, seg.inner.l, seg.inner.r);
			}
		}
	}

	auto total_inner_w = 0;

	for (int i = 0; i < num_segments; ++i) {
		segments[i].inner_offset = total_inner_w;
		total_inner_w += static_cast<int>(segments[i].inner.w());
	}

	/*
		Each segment covers its slices of the value range, regardless
		of the pixel leftover widening the first one - so a value landing
		exactly on a boundary fills its segments precisely to their edges,
		without a single pixel spilling into the next segment.
	*/

	auto local_ratio_of = [num_slices](const float global_ratio, const bar_segment& seg) {
		return std::clamp((global_ratio * num_slices - seg.slice_index) / seg.slice_count, 0.0f, 1.0f);
	};

	/*
		A segment terminated by the label brick maps its value range
		onto its visible span - the fill's moving edge thus always
		shows up before the brick, never hiding underneath.
	*/

	for (int i = 0; i < num_segments; ++i) {
		auto& seg = segments[i];
		const auto mapped_w = seg.open_right ? seg.visible_r - seg.inner.l : seg.inner.w();

		seg.fill_w = mapped_w * local_ratio_of(clamped_ratio, seg);
	}

	if (particles != nullptr) {
		const auto inner_h = static_cast<int>(segments[0].inner.h());

		::advance_hud_bar_particles(*particles, total_secs, vec2i(total_inner_w, inner_h));
	}

	for (int i = 0; i < num_segments; ++i) {
		const auto& seg = segments[i];
		const auto inner = seg.inner;
		const auto visible_r = seg.visible_r;

		if (visible_r - inner.l < 1.0f) {
			/* Entirely hidden under the label brick. */
			continue;
		}

		auto fill_rect = inner;
		fill_rect.w(std::clamp(seg.fill_w, 0.0f, visible_r - inner.l));

		/*
			The full black frame: an outline outside the bright border
			(always 1px, whatever the bright border's width),
			plus the 1px gap between the bright border and the bar itself.
			Drawn as two rings so that the bar's interior stays translucent.
			With split_gap == 0, the neighboring segments' outer rings
			land on the same pixel column, sharing a single outline.
		*/

		if (!seg.open_right) {
			if (appearance.black_frame) {
				output.border(inner, black, border_input { 1, border_expansion });
				output.border(inner, black, border_input { 1, 0 });
			}

			if (appearance.dark_background) {
				output.aabb(inner, dark_bar_col);
			}

			output.aabb(fill_rect, bar_col);
			output.border(inner, bar_col, bar_border);
		}
		else {
			/*
				An open right end: the frames' top, bottom and left lines
				run up to the brick, with no right lines at all -
				the brick terminates the segment in their place.
			*/

			const auto frame_cut = *bar_truncate_r;

			auto open_frame = [&](const ltrb frame, const int thickness, const rgba col) {
				const auto th = static_cast<float>(thickness);

				auto piece = [&](ltrb line) {
					line.r = std::min(line.r, frame_cut);

					if (line.w() >= 1) {
						output.aabb(line, col);
					}
				};

				piece(ltrb(frame.l, frame.t, frame.r, frame.t + th));
				piece(ltrb(frame.l, frame.b - th, frame.r, frame.b));
				piece(ltrb(frame.l, frame.t + th, frame.l + th, frame.b - th));
			};

			if (appearance.black_frame) {
				open_frame(ltrb(inner).expand_from_center(vec2::square(static_cast<float>(border_expansion + 1))), 1, black);
				open_frame(ltrb(inner).expand_from_center(vec2::square(1)), 1, black);
			}

			if (appearance.dark_background) {
				output.aabb(ltrb(inner.l, inner.t, visible_r, inner.b), dark_bar_col);
			}

			output.aabb(fill_rect, bar_col);
			open_frame(ltrb(inner).expand_from_center(vec2::square(static_cast<float>(border_expansion))), appearance.border_w, bar_col);
		}

		/*
			The fill's moving right edge: its last pixel column,
			followed by a 1px outline in the same shade/alpha the background fill uses.
			Skipped in the segments the fill (nearly) fills up or misses entirely.
		*/

		if (appearance.fill_edge_outline && fill_rect.w() >= 1) {
			const auto fill_r = fill_rect.r;

			if (fill_r + 1 <= visible_r) {
				const auto edge_col = bar_col;//;augs::interp(bar_col, particle_col, 0.25f);

				output.aabb(ltrb(fill_r - 1, inner.t, fill_r, inner.b), edge_col);
				output.aabb(ltrb(fill_r, inner.t, fill_r + 1, inner.b), dark_bar_col);
			}
		}

		if (particles != nullptr) {
			const auto field_origin = vec2(inner.l - seg.inner_offset, inner.t);

			::draw_hud_bar_particles(*particles, output, necessary_images, field_origin, fill_rect, particle_col);
		}
	}

	/*
		The white flash on increase - the same timing
		as the white highlight of a damaged character's health bar,
		scaled by the duration multiplier. The flashed range lives
		in the concatenated inner space, clipped to each segment's interior.
	*/

	if (highlight.at_secs >= 0.0) {
		const auto highlight_secs = highlight_base_duration_secs * bar_increase_highlight_duration_mult_v;
		const auto passed = total_secs - highlight.at_secs;

		if (passed >= 0.0 && passed < highlight_secs) {
			auto highlight_col = white;
			highlight_col.mult_alpha(1.0f - static_cast<float>(passed / highlight_secs));

			const auto flashed_range = [&]() -> std::pair<float, float> {
				if constexpr(bar_increase_highlight_only_added_part_v) {
					return { highlight.from_ratio, highlight.to_ratio };
				}

				return { 0.0f, clamped_ratio };
			}();

			for (int i = 0; i < num_segments; ++i) {
				const auto& seg = segments[i];
				const auto mapped_w = seg.open_right ? seg.visible_r - seg.inner.l : seg.inner.w();

				const auto local_from = mapped_w * local_ratio_of(flashed_range.first, seg);
				const auto local_to = mapped_w * local_ratio_of(flashed_range.second, seg);

				if (local_from < local_to) {
					output.aabb(ltrb(seg.inner.l + local_from, seg.inner.t, seg.inner.l + local_to, seg.inner.b), highlight_col);
				}

				if (bar_increase_highlight_affects_border_v && !seg.open_right) {
					output.border(seg.inner, highlight_col, bar_border);
				}
			}
		}
	}

	if (label_font != nullptr && !appearance.label.empty()) {
		using namespace augs::gui::text;

		auto label_col = appearance.label_color.value_or(particle_col);
		label_col.a = 255;

		const auto label_fs = formatted_string(appearance.label, style(*label_font, label_col));
		auto label_center = vec2i(bordered_rect.get_center());

		std::optional<vec2i> right_aligned_text_pos;

		if (appearance.label_background) {
			const auto label_bw = std::max(1, appearance.label_border_w);
			const auto label_bbox = get_text_bbox(label_fs);

			/*
				A thicker frame eats into the padding, so that the whole block
				stays the same size as with the original 1px border -
				the neighboring bars' labels must not push into each other.
			*/

			const auto padding = vec2(appearance.label_padding) - vec2::square(2.0f * label_bw);

			/* The dark 1px inner outline, the luminous border band, then the black 1px outline. */
			const auto frame_expansion = 1 + label_bw + 1;

			/*
				All the edges land on whole pixels - a backdrop centered
				with an odd size would put its frame on half-pixel
				coordinates, rasterizing the lines unevenly.
			*/

			const auto backdrop_size = [&]() {
				auto result = vec2i(vec2(label_bbox) + padding);

				if (appearance.label_align_right && !appearance.label_widest_text.empty()) {
					/*
						A constant width fitting the widest possible text,
						so the labels of stacked bars line up in a column.
					*/

					const auto widest_bbox = get_text_bbox(formatted_string(appearance.label_widest_text, style(*label_font, label_col)));

					result.x = widest_bbox.x + static_cast<int>(padding.x);
				}

				return result;
			}();

			const auto backdrop_l = [&]() {
				if (appearance.label_align_right) {
					/*
						The block's whole frame ends exactly at the bar's outermost
						right edge - including the bar's 1px black outline,
						so the last pixel columns of both land at the same x.
					*/
					const auto bar_outer_r = static_cast<int>(bordered_rect.r) + (appearance.black_frame ? 1 : 0);

					return bar_outer_r - frame_expansion - backdrop_size.x;
				}

				return label_center.x - backdrop_size.x / 2;
			}();

			const auto backdrop_t = [&]() {
				if (appearance.label_align_bottom) {
					/* The block's whole frame ends exactly at the bar's bottom edge. */
					return static_cast<int>(bordered_rect.b) - frame_expansion - backdrop_size.y;
				}

				if (appearance.label_reveal_bottom_px.has_value()) {
					const auto inner_b = static_cast<int>(bordered_rect.b) - border_expansion;

					return inner_b - *appearance.label_reveal_bottom_px - frame_expansion - backdrop_size.y;
				}

				return label_center.y - backdrop_size.y / 2;
			}();

			const auto backdrop_rect = ltrb(vec2(vec2i(backdrop_l, backdrop_t)), vec2(backdrop_size));

			label_center = vec2i(backdrop_rect.get_center());

			if (appearance.label_align_right) {
				right_aligned_text_pos = vec2i(
					static_cast<int>(backdrop_rect.r - std::max(0.0f, padding.x / 2)) - label_bbox.x,
					label_center.y
				);
			}

			/*
				The frame: the luminous border band right around the backdrop,
				with a black 1px outline outside it.
				Every line can be scissored horizontally to a given x range.
			*/

			auto frame_sides = [&](const ltrb frame, const int thickness, const rgba col, const std::optional<float> cut_from_x, const std::optional<float> cut_at_x) {
				const auto th = static_cast<float>(thickness);

				auto piece = [&](ltrb line) {
					if (cut_from_x.has_value()) {
						line.l = std::max(line.l, *cut_from_x);
					}

					if (cut_at_x.has_value()) {
						line.r = std::min(line.r, *cut_at_x);
					}

					if (line.w() >= 1) {
						output.aabb(line, col);
					}
				};

				piece(ltrb(frame.l, frame.t, frame.r, frame.t + th));
				piece(ltrb(frame.l, frame.b - th, frame.r, frame.b));
				piece(ltrb(frame.l, frame.t + th, frame.l + th, frame.b - th));
				piece(ltrb(frame.r - th, frame.t + th, frame.r, frame.b - th));
			};

			const auto inner_outline = ltrb(backdrop_rect).expand_from_center(vec2::square(1));
			const auto border_band = ltrb(backdrop_rect).expand_from_center(vec2::square(1 + label_bw));
			const auto black_outline = ltrb(backdrop_rect).expand_from_center(vec2::square(2 + label_bw));

			/*
				One dark quad under the whole block - the text area
				and the border bands alike - so that wherever a frame line
				gets scissored away, the background shade shows in its place.
			*/

			output.aabb(border_band, appearance.label_background_color.value_or(dark_bar_col));

			/*
				The x the given value ratio maps to on the border:
				either the covered bar's actual fill edge (across its segments),
				or - with the full range sweep - a linear map onto
				the band's whole width.
			*/

			const auto border_x_of_ratio = [&](const float of_ratio) {
				if (appearance.label_border_full_range_sweep) {
					return border_band.l + border_band.w() * std::clamp(of_ratio, 0.0f, 1.0f);
				}

				const auto raw_fill_x = [&]() {
					for (int i = 0; i < num_segments; ++i) {
						const auto& seg = segments[i];
						const auto local = local_ratio_of(of_ratio, seg);

						if (local < 1.0f || i == num_segments - 1) {
							return seg.inner.l + seg.inner.w() * local;
						}
					}

					return segments[num_segments - 1].inner.r;
				}();

				/*
					The band can stick out past the fill's maximum extent
					(e.g. when right-aligned) - over the last stretch of the fill,
					the mapping catches up with the overhang, so the band's far
					columns sweep in smoothly instead of popping in at 100%.
				*/

				auto result = raw_fill_x;

				const auto fill_max_x = segments[num_segments - 1].inner.r;
				const auto overhang = border_band.r - fill_max_x;

				if (overhang > 0.0f) {
					const auto window = std::max(3.0f * overhang, 1.0f);
					const auto catch_up = std::clamp((raw_fill_x - (fill_max_x - window)) / window, 0.0f, 1.0f);

					result += overhang * catch_up;
				}

				return std::clamp(result, border_band.l, border_band.r);
			};

			/*
				A full bar always draws the whole border - the band can reach
				past the fill's maximum extent (e.g. when right-aligned),
				where the fill-edge cut would never let it complete.
			*/

			const auto luminous_cut = [&]() -> std::optional<float> {
				if (!appearance.label_border_shows_value || clamped_ratio >= 1.0f) {
					return std::nullopt;
				}

				return border_x_of_ratio(clamped_ratio);
			}();

			/*
				The dark 1px outline inside the luminous border,
				optionally swept by the value just like the border itself.
			*/

			frame_sides(inner_outline, 1, black, std::nullopt, appearance.label_inner_outline_shows_value ? luminous_cut : std::nullopt);

			frame_sides(border_band, label_bw, bar_col, std::nullopt, luminous_cut);

			/*
				The unfilled remainder: a thin luminous outline at the band's
				outer edge, with no dark inner outline in that part.
			*/

			if (luminous_cut.has_value() && appearance.label_unfilled_border_w > 0) {
				const auto thin_w = appearance.label_unfilled_border_w;

				frame_sides(border_band, thin_w, bar_col, *luminous_cut, std::nullopt);

				/*
					The left column must never vanish: with the sweep sitting
					inside it, neither the clipped thick band nor the clipped
					thin ring would draw it whole - so draw it explicitly.
				*/

				if (*luminous_cut < border_band.l + thin_w) {
					const auto th = static_cast<float>(thin_w);

					output.aabb(ltrb(border_band.l, border_band.t + th, border_band.l + th, border_band.b - th), bar_col);
				}
			}

			/*
				The white flash mirrored onto the luminous border,
				over the same value range the bar itself flashes with.
			*/

			if (highlight.at_secs >= 0.0) {
				const auto highlight_secs = highlight_base_duration_secs * bar_increase_highlight_duration_mult_v;
				const auto passed = total_secs - highlight.at_secs;

				if (passed >= 0.0 && passed < highlight_secs) {
					auto highlight_col = white;
					highlight_col.mult_alpha(1.0f - static_cast<float>(passed / highlight_secs));

					const auto flashed_range = [&]() -> std::pair<float, float> {
						if constexpr(bar_increase_highlight_only_added_part_v) {
							return { border_x_of_ratio(highlight.from_ratio), border_x_of_ratio(highlight.to_ratio) };
						}

						return { border_band.l, border_x_of_ratio(clamped_ratio) };
					}();

					frame_sides(border_band, label_bw, highlight_col, flashed_range.first, flashed_range.second);
				}
			}

			frame_sides(black_outline, 1, black, std::nullopt, std::nullopt);
		}

		if (right_aligned_text_pos.has_value()) {
			print_stroked(
				output,
				*right_aligned_text_pos,
				label_fs,
				{ augs::ralign::CY }
			);
		}
		else {
			print_stroked(
				output,
				label_center,
				label_fs,
				{ augs::ralign::CX, augs::ralign::CY }
			);
		}
	}
}
