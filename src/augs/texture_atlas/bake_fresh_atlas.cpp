#include <string>
#include <sstream>
#include <numeric>

#include "3rdparty/rectpack2D/src/finders_interface.h"

#include "augs/ensure.h"
#include "augs/misc/measurements.h"

#include "augs/readwrite/memory_stream.h"

#include "augs/image/image.h"
#include "augs/image/blit.h"
#include "augs/texture_atlas/bake_fresh_atlas.h"

#include "augs/readwrite/byte_file.h"
#include "augs/filesystem/directory.h"
#include "augs/templates/range_workers.h"

#define DEBUG_FILL_IMGS_WITH_COLOR 0
#define TEST_SAVE_ATLAS 0

#if DEBUG_FILL_IMGS_WITH_COLOR
#include "augs/misc/randomization.h"
#endif

using namespace rectpack2D;

void bake_fresh_atlas(
	const bake_fresh_atlas_input in,
	const bake_fresh_atlas_output out
) {
	const auto& subjects = in.subjects;

	auto& baked = out.baked;
	auto& output_image_size = out.baked.atlas_image_size;

	std::unordered_map<source_font_identifier, augs::font> loaded_fonts;

	thread_local std::vector<rect_xywhf> rects_for_packer;
	rects_for_packer.clear();
	rects_for_packer.reserve(subjects.count_images());

#if DEBUG_FILL_IMGS_WITH_COLOR
	thread_local randomization rng;
#endif

	{
		auto scope = measure_scope_additive(out.profiler.loading_image_sizes);

		for (const auto& input_img_id : subjects.images) {
			auto& out_entry = baked.images[input_img_id];

			try {
				const auto u_size = [&]() { 
					auto sc = measure_scope(scope); 
					return augs::image::get_size(input_img_id); 
				}();

				out_entry.cached_original_size_pixels = u_size;

				const auto size = vec2i(u_size);
				rects_for_packer.push_back(rect_xywh(0, 0, size.x, size.y));
			}
			catch (const augs::image_loading_error& err) {
				out_entry.cached_original_size_pixels = vec2u::zero;
				rects_for_packer.push_back(rect_xywh(0, 0, 0, 0));
			}
		}

		baked.loaded_pngs.resize(subjects.loaded_pngs.size());

		for (const auto& input_png_bytes : subjects.loaded_pngs) {
			const auto input_img_id = index_in(subjects.loaded_pngs, input_png_bytes);

			auto& out_entry = baked.loaded_pngs[input_img_id];

			try {
				const auto u_size = [&]() { 
					auto sc = measure_scope(scope); 
					return augs::image::get_png_size(input_png_bytes); 
				}();

				out_entry.cached_original_size_pixels = u_size;

				const auto size = vec2i(u_size);
				rects_for_packer.push_back(rect_xywh(0, 0, size.x, size.y));
			}
			catch (const augs::image_loading_error& err) {
				out_entry.cached_original_size_pixels = vec2u::zero;
				rects_for_packer.push_back(rect_xywh(0, 0, 0, 0));
			}
		}
	}

	{
		auto scope = measure_scope(out.profiler.loading_fonts);

		for (const auto& input_fnt_id : subjects.fonts) {
			const auto it = loaded_fonts.try_emplace(input_fnt_id, input_fnt_id);

			{
				const bool is_font_unique = it.second;
				(void)is_font_unique;
				ensure(is_font_unique);
			}

			const auto& fnt = (*it.first).second;

			auto& out_fnt = baked.fonts[input_fnt_id];
			out_fnt.meta = fnt.meta;

			for (const auto& g : fnt.glyph_bitmaps) {
				out_fnt.glyphs_in_atlas.push_back({});

				auto& out_entry = *out_fnt.glyphs_in_atlas.rbegin();

				out_entry.cached_original_size_pixels = g.get_size();

				rects_for_packer.push_back(rect_xywh(
					0,
					0,
					static_cast<int>(g.get_size().x),
					static_cast<int>(g.get_size().y)
				));
			}

#if DEBUG_FILL_IMGS_WITH_COLOR
			for (auto& img : (*it.first).second.glyph_bitmaps) {
				img.fill(rgba(white).set_hsv({ rng.randval(0.0f, 1.0f), rng.randval(0.3f, 1.0f), rng.randval(0.3f, 1.0f) }));
			}
#endif
		}
	}

	{
		auto scope = measure_scope(out.profiler.packing);

		const auto max_size = static_cast<int>(in.max_atlas_size);
		const auto rect_padding_amount = 2;

		out.profiler.subjects_count.measure(rects_for_packer.size());

		for (auto& rr : rects_for_packer) {
			rr.w += rect_padding_amount;
			rr.h += rect_padding_amount;
		}

		constexpr bool allow_flip = true;

		using spaces_type = rectpack2D::empty_spaces<allow_flip>;

		const auto result_size = find_best_packing<spaces_type>(
			rects_for_packer,
			make_finder_input(
				max_size,
				1,
				[](auto){ return rectpack2D::callback_result::CONTINUE_PACKING; },
				[](const auto& r){ LOG("ERROR: (%x;%x;%x;%x) didn't fit into atlas.", r.x, r.y, r.w, r.h); return rectpack2D::callback_result::ABORT_PACKING; },
				flipping_option::ENABLED
			)
		);

		output_image_size = vec2u(result_size.w, result_size.h);

		std::size_t total_used_space = 0;

		for (auto& rr : rects_for_packer) {
			total_used_space += rr.area();
		}

		for (auto& rr : rects_for_packer) {
			rr.w -= rect_padding_amount;
			rr.h -= rect_padding_amount;
		}

		out.profiler.atlas_size.measure(output_image_size);
		const auto wasted_space = output_image_size.area() - total_used_space;
		out.profiler.wasted_space.measure(wasted_space);
		out.profiler.wasted_space_percent.measure(100 * double(wasted_space) / output_image_size.area());
		//out.profiler.atlas_width.measure(output_image_size.x);
	}

	auto output_image = [&out, output_image_size]() {
		if (out.whole_image != nullptr) {
			return augs::image_view(out.whole_image, output_image_size);
		}
		else {
			out.fallback_output.resize(output_image_size.area());
			return augs::image_view(out.fallback_output.data(), output_image_size);
		}
	}();

#if DEBUG_FILL_IMGS_WITH_COLOR
	output_image.fill({0, 0, 0, 255});
#endif

	{
		thread_local std::vector<std::vector<std::byte>> all_loaded_bytes;

		{
			auto scope = measure_scope(out.profiler.loading_images);

			const auto images_n = subjects.images.size();

				if (images_n > all_loaded_bytes.size()) {
					all_loaded_bytes.resize(images_n);
				}

			for (const auto& input_img_id : subjects.images) {
				const auto current_rect = index_in(subjects.images, input_img_id);

				try {
					all_loaded_bytes[current_rect].clear();
					augs::file_to_bytes(input_img_id, all_loaded_bytes[current_rect]);
				}
				catch (...) {

				}
			}
		}

		struct worker_input {
			unsigned image_area;
			unsigned original_index;

			worker_input() {}

			bool operator<(const worker_input& b) const {
				/* Biggest go first */
				return image_area > b.image_area;
			}
		};

		thread_local std::vector<worker_input> worker_inputs;

		{
			auto scope = measure_scope(out.profiler.making_worker_inputs);
			worker_inputs.resize(subjects.count_images());

			for (const auto& r : subjects.images) {
				const auto current_rect = static_cast<unsigned>(index_in(subjects.images, r));
				const auto area = static_cast<unsigned>(rects_for_packer[current_rect].area());

				worker_inputs[current_rect].image_area = area;
				worker_inputs[current_rect].original_index = current_rect;
			}

			for (const auto& r : subjects.loaded_pngs) {
				const auto current_rect = subjects.images.size() + static_cast<unsigned>(index_in(subjects.loaded_pngs, r));
				const auto area = static_cast<unsigned>(rects_for_packer[current_rect].area());

				worker_inputs[current_rect].image_area = area;
				worker_inputs[current_rect].original_index = current_rect;
			}

			sort_range(worker_inputs);
		}

		auto scope = measure_scope(out.profiler.blitting_images);

		auto worker = [&output_image, &subjects, &baked, output_image_size](const worker_input& input) {
			const bool is_loaded_png = input.original_index >= subjects.images.size();
			const auto loaded_png_index = input.original_index - subjects.images.size();

			const auto current_rect = input.original_index;

			const auto& input_img_id = 
				is_loaded_png ? 
				augs::path_type() : 
				subjects.images[current_rect]
			;

			const auto packed_rect = rects_for_packer[current_rect];

			auto& output_entry = is_loaded_png ? baked.loaded_pngs[loaded_png_index] : baked.images[input_img_id];
			const auto& error_reported_img_id = input_img_id;

			auto set_glitch_uv = [&output_entry, output_image_size](){
				output_entry.atlas_space.set(0.f, 0.f, 1.f, 1.f);
				output_entry.cached_original_size_pixels = output_image_size;
				output_entry.was_flipped = false;
				output_entry.was_successfully_packed = false;
			};

			if (output_entry.cached_original_size_pixels.is_zero()) {
				/* 
					Image failed to load from disk. 

					Set the texture coordinate to the entire atlas, 
					so that the glitch is immediately noticeable.
				*/

				set_glitch_uv();
				return;
			}

			output_entry.atlas_space.set(
				static_cast<float>(packed_rect.x + 1) / output_image_size.x,
				static_cast<float>(packed_rect.y + 1) / output_image_size.y,
				static_cast<float>(packed_rect.w) / output_image_size.x,
				static_cast<float>(packed_rect.h) / output_image_size.y
			);

			output_entry.was_flipped = packed_rect.flipped;
			output_entry.was_successfully_packed = true;

			thread_local augs::image loaded_png;

			const auto& source_bytes = 
				is_loaded_png ?
				subjects.loaded_pngs[loaded_png_index] :
				all_loaded_bytes[current_rect]
			;

			if (source_bytes.empty()) {
				set_glitch_uv();
				return;
			}

			try {
				loaded_png.from_png(source_bytes, error_reported_img_id);
			}
			catch (...) {
				set_glitch_uv();
				return;
			}

#if DEBUG_FILL_IMGS_WITH_COLOR
			loaded_png.fill(rgba(white).set_hsv({ rng.randval(0.0f, 1.0f), rng.randval(0.3f, 1.0f), rng.randval(0.3f, 1.0f) }));
#endif
			augs::blit(
				output_image,
				loaded_png,
				{
					static_cast<unsigned>(packed_rect.x + 1),
					static_cast<unsigned>(packed_rect.y + 1)
				},
				packed_rect.flipped
			);

			augs::blit_border(
				output_image,
				loaded_png,
				{
					static_cast<unsigned>(packed_rect.x + 1),
					static_cast<unsigned>(packed_rect.y + 1)
				},
				packed_rect.flipped
			);
		};

#if 1
		for (const auto& w : worker_inputs) {
			worker(w);
		}
#else
		const auto num_workers = std::size_t(in.blitting_threads - 1);

		thread_local augs::range_workers<decltype(worker)> workers = num_workers;
		workers.resize_workers(num_workers);
		workers.process(worker, worker_inputs);
#endif
	}

	{
		auto scope = measure_scope(out.profiler.blitting_fonts);

		std::size_t current_rect = subjects.images.size();

		for (auto& input_font_id : subjects.fonts) {
			auto& output_font = baked.fonts[input_font_id];

			const auto n = output_font.glyphs_in_atlas.size();

			for (auto& g : output_font.glyphs_in_atlas) {
				const auto glyph_index = index_in(output_font.glyphs_in_atlas, g);
				const auto& packed_rect = rects_for_packer[current_rect + glyph_index];

				g.atlas_space.set(
					static_cast<float>(packed_rect.x) / output_image_size.x,
					static_cast<float>(packed_rect.y) / output_image_size.y,
					static_cast<float>(packed_rect.w) / output_image_size.x,
					static_cast<float>(packed_rect.h) / output_image_size.y
				);

				g.was_flipped = packed_rect.flipped;

				augs::blit(
					output_image,
					loaded_fonts.at(input_font_id).glyph_bitmaps[glyph_index],
					{
						static_cast<unsigned>(packed_rect.x),
						static_cast<unsigned>(packed_rect.y)
					},
					packed_rect.flipped
				);
			}

			current_rect += n;
		}
	}

#if TEST_SAVE_ATLAS
	augs::image(output_image.get_data(), output_image.get_size()).save_as_png("/tmp/atl.png");
#endif
}