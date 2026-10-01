#include <cstddef>
#include <array>
#include <limits>
#include <optional>
#include <algorithm>

#include "augs/math/vec2.h"
#include "augs/math/arithmetical.h"
#include "augs/drawing/drawing.hpp"
#include "augs/templates/container_templates.h"
#include "augs/gui/text/printer.h"
#include "view/hud_bar_drawing.h"

/*
	The flash lasts this much longer than the white highlight
	of a damaged character's health bar.
*/
constexpr float bar_increase_highlight_duration_mult_v = 2.0f;

constexpr std::size_t max_hud_bar_flashes_v = 16;

/* Gains closer together than this in time merge into a single flash. */
constexpr double continuous_growth_max_gap_secs_v = 0.06;

/* The same 15 Hz wandering the character's value bars always had. */
constexpr double hud_bar_particle_step_secs_v = 1.0 / 15;

/*
	A longer gap is not replayed - the particles just continue from now.
	Otherwise, joining a match in progress or a demo seek forward
	would first simulate every step since the cosmos' start.
*/
constexpr double hud_bar_particle_max_catch_up_secs_v = 0.25;

static void update_hud_bar_highlight(
	hud_bar_highlight_state& highlight,
	const float ratio,
	const double total_secs,
	const double flash_duration_secs,
	const bool multiple_flashes,
	const bool flash_whole_on_full
) {
	if (total_secs < highlight.last_update_secs) {
		/*
			The cosmos' clock went back - a round restart or a demo seek.
			Start tracking afresh: the jump back to the initial values
			must not read as a gain, and the pending flashes whose timestamps
			come from the previous timeline must not replay.
		*/

		highlight.flashes.clear();
		highlight.last_ratio = ratio;
	}

	highlight.last_update_secs = total_secs;

	erase_if(highlight.flashes, [&](const hud_bar_flash& flash) {
		return total_secs - flash.at_secs >= flash_duration_secs;
	});

	if (const bool just_filled = flash_whole_on_full && ratio >= 1.0f && highlight.last_ratio >= 0.0f && highlight.last_ratio < 1.0f) {
		/* Seen as growing from zero, so the flash covers the whole bar. */
		highlight.last_ratio = 0.0f;
	}

	if (highlight.last_ratio >= 0.0f && ratio > highlight.last_ratio) {
		auto& flashes = highlight.flashes;

		if (!multiple_flashes) {
			flashes.clear();
			flashes.push_back({ highlight.last_ratio, ratio, total_secs });
		}
		else {
			/*
				A continuously growing value merges into a single flash
				covering the whole run - it stays lit while the growth lasts
				and fades out once it pauses. Otherwise, per-frame micro-flashes
				would flood the list and evict each other before finishing.
				Gains separated in time still get their own independent flashes.
			*/

			const bool continues_the_last_flash =
				!flashes.empty()
				&& flashes.back().to_ratio >= highlight.last_ratio - 0.0001f
				&& total_secs - flashes.back().at_secs < continuous_growth_max_gap_secs_v
			;

			if (continues_the_last_flash) {
				flashes.back().to_ratio = ratio;
				flashes.back().at_secs = total_secs;
			}
			else {
				if (flashes.size() >= max_hud_bar_flashes_v) {
					flashes.erase(flashes.begin());
				}

				flashes.push_back({ highlight.last_ratio, ratio, total_secs });
			}
		}
	}

	highlight.last_ratio = ratio;
}

/*
	The same flowing particles the character's value bars draw
	inside their filled parts. The particles live in a virtual field
	spanning the concatenation of all the segments' interiors,
	so they flow continuously across the split borders.
*/

static void advance_hud_bar_particles(
	hud_bar_particles_state& state,
	const double total_secs,
	const vec2i field_size,
	const int px_per_particle = 4
) {
	if (field_size.x <= 0 || field_size.y <= 0) {
		return;
	}

	const bool respawn = state.particles.empty() || state.spawned_for_size != field_size;

	if (respawn) {
		state.particles.clear();
		state.spawned_for_size = field_size;

		thread_local randomization seed_rng;
		state.rng = randomization(static_cast<rng_seed_type>(seed_rng.randval(0, std::numeric_limits<int>::max())));

		/* The default density is the character's value bars': 40 particles per a 160px bar. */
		const auto num_particles_to_spawn = static_cast<std::size_t>(std::max(1, field_size.x / std::max(1, px_per_particle)));

		const auto mats = std::array<assets::necessary_image_id, 3> {
			assets::necessary_image_id::WANDERING_CROSS,
			assets::necessary_image_id::BLINK_1,
			static_cast<assets::necessary_image_id>(static_cast<int>(assets::necessary_image_id::BLINK_1) + 2),
		};

		for (std::size_t i = 0; i < num_particles_to_spawn; ++i) {
			auto new_part = hud_bar_particle();
			new_part.relative_pos = state.rng.randval(vec2(0, 0), vec2(field_size));
			new_part.image_id = state.rng.choose_from(mats);

			state.particles.push_back(new_part);
		}
	}

	const auto since_last_advance = total_secs - state.last_advance_secs;

	if (respawn || since_last_advance < 0.0 || since_last_advance > hud_bar_particle_max_catch_up_secs_v) {
		state.last_advance_secs = total_secs;
	}

	auto& rng = state.rng;

	while (total_secs - state.last_advance_secs > hud_bar_particle_step_secs_v) {
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

		state.last_advance_secs += hud_bar_particle_step_secs_v;
	}
}

static void draw_hud_bar_particles(
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
	The lines of a rectangular frame, each scissored horizontally
	to the given x range. Without the right line, the frame stays open
	on its right - whatever lies there terminates it instead.
*/

static void draw_frame_lines(
	const augs::drawer_with_default output,
	const ltrb frame,
	const int thickness,
	const rgba col,
	const std::optional<float> cut_from_x,
	const std::optional<float> cut_at_x,
	const bool with_right_line = true
) {
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

	if (with_right_line) {
		piece(ltrb(frame.r - th, frame.t + th, frame.r, frame.b - th));
	}
}

void draw_hud_bar(
	const hud_bar_draw_input& in,
	hud_bar_state& state,
	const hud_bar_appearance& appearance,
	const ltrb bordered_rect,
	const float ratio
) {
	using namespace augs::gui::text;

	const auto output = in.output;
	const auto total_secs = in.total_secs;
	auto& highlight = state.highlight;

	const auto bar_col = appearance.color;
	const auto clamped_ratio = std::clamp(ratio, 0.0f, 1.0f);
	const auto flash_secs = static_cast<double>(in.highlight_base_duration_secs * bar_increase_highlight_duration_mult_v);

	::update_hud_bar_highlight(highlight, clamped_ratio, total_secs, flash_secs, appearance.multiple_flashes, appearance.flash_whole_on_full);

	const auto bar_border = border_input { appearance.border_w, 1 };
	const auto border_expansion = bar_border.get_total_expansion();

	/* Includes the 1px black outline around the bright border. */
	const auto bar_outer_r = static_cast<int>(bordered_rect.r) + 1;

	auto dark_bar_col = rgba(bar_col) * 0.4f;
	dark_bar_col.a = 200;

	auto particle_col = augs::interp(white, bar_col, appearance.particle_tint);
	particle_col.a = 220;

	auto label_col = particle_col;
	label_col.a = 255;

	const bool has_label = !appearance.label.empty();
	const auto label_fs = formatted_string(appearance.label, style(in.font, label_col));
	const auto label_bbox = has_label ? get_text_bbox(label_fs) : vec2i();

	/*
		The label block's frame: the dark 1px inner outline,
		the luminous border band, then the black 1px outline.
		The band eats into the padding, so that the whole block
		keeps the same size whatever the band's width -
		the neighboring bars' labels must not push into each other.
	*/

	const auto label_bw = std::max(1, appearance.label_border_w);
	const auto label_frame_expansion = 1 + label_bw + 1;
	const auto backdrop_padding = vec2(appearance.label_padding) - vec2::square(2.0f * label_bw);

	/* A right-aligned label reserves the width of the widest possible text. */

	const auto label_column_text_w = [&]() {
		if (has_label && appearance.label_align_right && !appearance.label_widest_text.empty()) {
			return get_text_bbox(formatted_string(appearance.label_widest_text, style(in.font, label_col))).x;
		}

		return label_bbox.x;
	}();

	const auto backdrop_size = vec2i(
		label_column_text_w + static_cast<int>(backdrop_padding.x),
		label_bbox.y + static_cast<int>(backdrop_padding.y)
	);

	/*
		A right-aligned label truncates the bar: nothing may render under
		the label's area - the bar (its frames included) ends right where
		the label's brick begins, or - with no backdrop - the plain text's
		reserved column. This is the x the bar's outermost right edge may reach.
	*/

	const auto bar_truncate_r = [&]() -> std::optional<float> {
		if (!has_label || !appearance.label_align_right) {
			return std::nullopt;
		}

		const auto reserved_w =
			appearance.label_background ?
			2 * label_frame_expansion + backdrop_size.x :
			label_column_text_w + appearance.label_padding.x
		;

		return static_cast<float>(bar_outer_r - reserved_w);
	}();

	/*
		The segment layout. A single bar is just one segment spanning
		the whole bordered_rect.
	*/

	struct bar_segment {
		ltrb inner;
		float fill_w = 0.0f;

		/* The width the segment's value range maps onto. */
		float mapped_w = 0.0f;

		/* Where the segment's interior begins in the concatenated inner space. */
		int inner_offset = 0;

		/* Interior pixels beyond visible_r hide under the label brick. */
		float visible_r = 0.0f;

		/* The right frame is not drawn - the brick terminates the segment. */
		bool open_right = false;
	};

	std::array<bar_segment, max_hud_bar_splits_v> segments;

	const auto num_segments = std::clamp(appearance.splits, 1, max_hud_bar_splits_v);
	const auto separation = num_segments > 1 ? (appearance.split_gap == 0 ? 1 : appearance.split_gap + 2) : 0;

	const auto total_w = static_cast<int>(bordered_rect.w());
	const auto segments_w = total_w - (num_segments - 1) * separation;

	const auto base_w = segments_w / num_segments;
	const auto leftover_w = segments_w % num_segments;

	{
		/* Every segment must keep at least a pixel of interior. */
		const auto min_segment_w = 2 * border_expansion + 1;

		if (base_w < min_segment_w || bordered_rect.h() < min_segment_w) {
			return;
		}
	}

	{
		const auto total_t = bordered_rect.t;
		const auto total_b = bordered_rect.b;

		auto x = static_cast<int>(bordered_rect.l);

		for (int i = 0; i < num_segments; ++i) {
			/* The division leftover spreads one pixel per segment, first ones first. */
			const auto this_w = base_w + (i < leftover_w ? 1 : 0);

			auto& seg = segments[i];
			seg.inner = ltrb(ltrb(x, total_t, x + this_w, total_b)).expand_from_center(vec2::square(static_cast<float>(-border_expansion)));
			seg.visible_r = seg.inner.r;

			x += this_w + separation;
		}
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

			const auto outer_r = seg.inner.r + border_expansion + 1;

			if (outer_r > touch_x) {
				if (appearance.label_background) {
					seg.open_right = true;
					seg.visible_r = std::clamp(touch_x, seg.inner.l, seg.inner.r);
				}
				else {
					/*
						A plain text label terminates nothing - close the segment,
						its full frame included, before the reserved column.
					*/
					seg.inner.r = std::max(seg.inner.l + 1, touch_x - border_expansion - 1);
					seg.visible_r = seg.inner.r;
				}
			}
		}
	}

	auto total_inner_w = 0;

	for (int i = 0; i < num_segments; ++i) {
		segments[i].inner_offset = total_inner_w;
		total_inner_w += static_cast<int>(segments[i].inner.w());
	}

	/*
		Each segment covers its slice of the value range, regardless
		of the pixel leftover widening the first ones - so a value landing
		exactly on a boundary fills its segments precisely to their edges,
		without a single pixel spilling into the next segment.
	*/

	auto local_ratio_of = [num_segments](const float global_ratio, const int segment_index) {
		return std::clamp(global_ratio * num_segments - segment_index, 0.0f, 1.0f);
	};

	/*
		A segment terminated by the label brick maps its value range
		onto its visible span - the fill's moving edge thus always
		shows up before the brick, never hiding underneath.
	*/

	for (int i = 0; i < num_segments; ++i) {
		auto& seg = segments[i];

		seg.mapped_w = seg.open_right ? seg.visible_r - seg.inner.l : seg.inner.w();
		seg.fill_w = seg.mapped_w * local_ratio_of(clamped_ratio, i);
	}

	::advance_hud_bar_particles(state.particles, total_secs, vec2i(total_inner_w, static_cast<int>(segments[0].inner.h())));

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

			An open right end draws no right lines at all -
			the brick terminates the segment in their place.
		*/

		if (!seg.open_right) {
			output.border(inner, black, border_input { 1, border_expansion });
			output.border(inner, black, border_input { 1, 0 });
			output.aabb(inner, dark_bar_col);
			output.aabb(fill_rect, bar_col);
			output.border(inner, bar_col, bar_border);
		}
		else {
			const auto frame_cut = *bar_truncate_r;

			auto open_frame = [&](const float expansion, const int thickness, const rgba col) {
				::draw_frame_lines(output, ltrb(inner).expand_from_center(vec2::square(expansion)), thickness, col, std::nullopt, frame_cut, false);
			};

			open_frame(static_cast<float>(border_expansion + 1), 1, black);
			open_frame(1.0f, 1, black);

			output.aabb(ltrb(inner.l, inner.t, visible_r, inner.b), dark_bar_col);
			output.aabb(fill_rect, bar_col);

			open_frame(static_cast<float>(border_expansion), appearance.border_w, bar_col);
		}

		/*
			The fill's moving right edge: its last pixel column,
			followed by a 1px outline in the same shade/alpha the background fill uses.
			Skipped in the segments the fill (nearly) fills up or misses entirely.
		*/

		if (fill_rect.w() >= 1) {
			const auto fill_r = fill_rect.r;

			if (fill_r + 1 <= visible_r) {
				output.aabb(ltrb(fill_r - 1, inner.t, fill_r, inner.b), bar_col);
				output.aabb(ltrb(fill_r, inner.t, fill_r + 1, inner.b), dark_bar_col);
			}
		}

		const auto field_origin = vec2(inner.l - seg.inner_offset, inner.t);

		::draw_hud_bar_particles(state.particles, output, in.necessary_images, field_origin, fill_rect, particle_col);
	}

	/*
		The white flash over the added part of the value - the same timing
		as the white highlight of a damaged character's health bar,
		scaled by the duration multiplier.
	*/

	auto for_each_fading_flash = [&](auto&& callback) {
		for (const auto& flash : highlight.flashes) {
			const auto passed = total_secs - flash.at_secs;

			if (passed >= 0.0 && passed < flash_secs) {
				auto highlight_col = white;
				highlight_col.mult_alpha(1.0f - static_cast<float>(passed / flash_secs));

				callback(flash, highlight_col);
			}
		}
	};

	/* The flashed range lives in the concatenated inner space, clipped to each segment's interior. */

	for_each_fading_flash([&](const hud_bar_flash& flash, const rgba highlight_col) {
		for (int i = 0; i < num_segments; ++i) {
			const auto& seg = segments[i];

			const auto local_from = seg.mapped_w * local_ratio_of(flash.from_ratio, i);
			const auto local_to = seg.mapped_w * local_ratio_of(flash.to_ratio, i);

			if (local_from < local_to) {
				output.aabb(ltrb(seg.inner.l + local_from, seg.inner.t, seg.inner.l + local_to, seg.inner.b), highlight_col);
			}
		}
	});

	if (!has_label) {
		return;
	}

	auto label_center = vec2i(bordered_rect.get_center());
	std::optional<vec2i> right_aligned_text_pos;

	if (appearance.label_background) {
		/*
			All the edges land on whole pixels - a backdrop centered
			with an odd size would put its frame on half-pixel
			coordinates, rasterizing the lines unevenly.
		*/

		const auto backdrop_l =
			appearance.label_align_right ?
			/*
				The block's whole frame ends exactly at the bar's outermost
				right edge - so the last pixel columns of both land at the same x.
			*/
			bar_outer_r - label_frame_expansion - backdrop_size.x :
			label_center.x - backdrop_size.x / 2
		;

		const auto backdrop_t =
			appearance.label_align_bottom ?
			static_cast<int>(bordered_rect.b) - label_frame_expansion - backdrop_size.y :
			label_center.y - backdrop_size.y / 2
		;

		const auto backdrop_rect = ltrb(vec2(vec2i(backdrop_l, backdrop_t)), vec2(backdrop_size));

		label_center = vec2i(backdrop_rect.get_center());

		if (appearance.label_align_right) {
			right_aligned_text_pos = vec2i(
				static_cast<int>(backdrop_rect.r - std::max(0.0f, backdrop_padding.x / 2)) - label_bbox.x,
				label_center.y
			);
		}

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
			The x the given value ratio maps to on the border.
		*/

		const auto border_x_of_ratio = [&](const float of_ratio) {
			/*
				A continuous linear map over the bar's whole span -
				deliberately ignoring the segmentation, so the border's
				sweep never jumps over the inter-segment frames
				at the slice boundaries.
			*/

			const auto fill_l = segments[0].inner.l;
			const auto fill_max_x = segments[num_segments - 1].inner.r;
			const auto raw_fill_x = fill_l + (fill_max_x - fill_l) * std::clamp(of_ratio, 0.0f, 1.0f);

			/*
				The band can stick out past the fill's maximum extent
				(e.g. when right-aligned) - over the last stretch of the fill,
				the mapping catches up with the overhang, so the band's far
				columns sweep in smoothly instead of popping in at 100%.
			*/

			auto result = raw_fill_x;

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
			if (clamped_ratio >= 1.0f) {
				return std::nullopt;
			}

			return border_x_of_ratio(clamped_ratio);
		}();

		/*
			The dark 1px outline inside the luminous border is drawn whole,
			in two parts: within the filled part it hugs the thick border,
			and within the unfilled part it moves out to touch the thin outline
			instead (the two lines may sit at different offsets across the sweep point).
		*/

		::draw_frame_lines(output, inner_outline, 1, black, std::nullopt, luminous_cut);

		if (luminous_cut.has_value()) {
			const auto thin_w = appearance.label_unfilled_border_w;
			const auto unfilled_expansion = std::max(1, 1 + label_bw - thin_w);
			const auto unfilled_outline = ltrb(backdrop_rect).expand_from_center(vec2::square(static_cast<float>(unfilled_expansion)));

			::draw_frame_lines(output, unfilled_outline, 1, black, *luminous_cut, std::nullopt);

			/*
				Only while the sweep sits INSIDE the filled ring's left
				column does that column (corner pixels included) turn
				fully luminous - no dark sliver ever cuts through
				the lit-up left edge at any percentage; once the sweep
				passes it, the dark column returns.
			*/

			if (*luminous_cut >= inner_outline.l && *luminous_cut < inner_outline.l + 1) {
				output.aabb(ltrb(inner_outline.l, inner_outline.t, inner_outline.l + 1, inner_outline.b), bar_col);
			}

			/*
				The completing connectors: short vertical black pixels
				at the sweep point, joining the filled part's dark
				outline with the unfilled part's one - the dark line
				thus runs continuously across the offset jump,
				along the top and the bottom edge alike.
			*/

			if (*luminous_cut > unfilled_outline.l && *luminous_cut < unfilled_outline.r - 1) {
				const auto connector_x = *luminous_cut;

				output.aabb(ltrb(connector_x, unfilled_outline.t, connector_x + 1, inner_outline.t + 1), black);
				output.aabb(ltrb(connector_x, inner_outline.b - 1, connector_x + 1, unfilled_outline.b), black);
			}
		}

		::draw_frame_lines(output, border_band, label_bw, bar_col, std::nullopt, luminous_cut);

		/*
			The unfilled remainder: a thin luminous outline at the band's
			outer edge, with no dark inner outline in that part.
		*/

		if (luminous_cut.has_value() && appearance.label_unfilled_border_w > 0) {
			const auto thin_w = appearance.label_unfilled_border_w;

			::draw_frame_lines(output, border_band, thin_w, bar_col, *luminous_cut, std::nullopt);

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
			A separate instance of the flowing particles, wandering
			inside the filled part of the luminous border - so the border
			evidently reads as lit up. Denser than the bar's own particles -
			only the thin ring of the whole wandering field is ever visible.
		*/

		{
			::advance_hud_bar_particles(state.label_particles, total_secs, vec2i(border_band.get_size()), 1);

			const auto label_particle_col = rgba(255, 255, 255, 220);
			const auto band_field_origin = vec2(border_band.l, border_band.t);
			const auto band_bw = static_cast<float>(label_bw);
			const auto lit_r = luminous_cut.has_value() ? *luminous_cut : border_band.r;

			const auto band_strips = std::array<ltrb, 4> {
				ltrb(border_band.l, border_band.t, lit_r, border_band.t + band_bw),
				ltrb(border_band.l, border_band.b - band_bw, lit_r, border_band.b),
				ltrb(border_band.l, border_band.t + band_bw, std::min(lit_r, border_band.l + band_bw), border_band.b - band_bw),
				ltrb(border_band.r - band_bw, border_band.t + band_bw, std::min(lit_r, border_band.r), border_band.b - band_bw)
			};

			for (const auto& strip : band_strips) {
				::draw_hud_bar_particles(state.label_particles, output, in.necessary_images, band_field_origin, strip, label_particle_col);
			}
		}

		/*
			The white flash mirrored onto the luminous border,
			over the same value range the bar itself flashes with.
		*/

		for_each_fading_flash([&](const hud_bar_flash& flash, const rgba highlight_col) {
			::draw_frame_lines(output, border_band, label_bw, highlight_col, border_x_of_ratio(flash.from_ratio), border_x_of_ratio(flash.to_ratio));
		});

		::draw_frame_lines(output, black_outline, 1, black, std::nullopt, std::nullopt);
	}
	else if (bar_truncate_r.has_value()) {
		/*
			A plain right-aligned label: the stroked text sits LEFT-aligned
			within the column the truncation reserved right of the bar, so all
			the values - the negative ones included - line up at the same x.
		*/

		right_aligned_text_pos = vec2i(static_cast<int>(*bar_truncate_r) + appearance.label_padding.x, label_center.y);
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
