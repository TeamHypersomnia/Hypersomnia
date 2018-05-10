#include <string>
#include <sstream>

#include "3rdparty/rectpack2D/src/finders_interface.h"

#include "augs/ensure.h"
#include "augs/misc/measurements.h"

#include "augs/readwrite/memory_stream.h"

#include "augs/image/image.h"
#include "augs/texture_atlas/atlas_generation.h"

#include "augs/readwrite/byte_file.h"
#include "augs/filesystem/directory.h"

#define DEBUG_FILL_IMGS_WITH_COLOR 0

#if DEBUG_FILL_IMGS_WITH_COLOR
#include "augs/misc/randomization.h"
#endif

using namespace rectpack2D;

baked_atlas::baked_atlas(regenerated_atlas_input in) {
	const auto& settings = in.settings;
	const auto& subjects = in.subjects;
	auto& output_image = in.output_image;

	const auto atlas_image_path = settings.regeneration_path;
	const auto atlas_metadata_path = augs::path_type(atlas_image_path).replace_extension(".meta");
	const auto atlas_stamp_path = augs::path_type(atlas_image_path).replace_extension(".stamp");

	bool should_regenerate = settings.force_regenerate;

	texture_atlas_stamp new_stamp;

	/* 
		If for some reason the integrity checking for atlas takes a long time, 
		we might disable it to improve performance. Unlikely, though.
	*/

	if (!settings.skip_source_image_integrity_check) {
		for (const auto& img_id : subjects.images) {
			/* 
				Acquire the reference now, so that if last_write_time fails, 
				we're at least left with the default value.

				Otherwise if a new image was added, but it couldn't be found,
				the stamp for this image would not be generated for proper comparison,
				and the atlas would assume that it is up-to-date.
			*/

			auto& stamp = new_stamp.image_stamps[img_id];

			try {
				stamp = augs::last_write_time(img_id);
			}
			catch (...) {

			}
		}

		for (const auto& fnt_id : subjects.fonts) {
			new_stamp.font_stamps[fnt_id] = augs::last_write_time(fnt_id.source_font_path);
		}

		if (!augs::exists(atlas_image_path)) {
			should_regenerate = true;
		}
		else {
			if (!augs::exists(atlas_stamp_path)) {
				should_regenerate = true;
			}
			else {
				const auto existent_stamp = augs::load_from_bytes<texture_atlas_stamp>(atlas_stamp_path);
				const bool stamps_match =
					existent_stamp.image_stamps == new_stamp.image_stamps
					&& existent_stamp.font_stamps == new_stamp.font_stamps
				;

				if (!stamps_match) {
					should_regenerate = true;
				}
			}
		}
	}

	if (should_regenerate) {
		LOG("Regenerating texture atlas: %x", atlas_image_path);
		
		std::unordered_map<source_image_identifier, augs::image> loaded_images;
		std::unordered_map<source_font_identifier, augs::font> loaded_fonts;

		std::vector<rect_xywhf> rects_for_packer;

#if DEBUG_FILL_IMGS_WITH_COLOR
		thread_local randomization rng;
#endif

		{
			auto scope = measure_scope(in.profiler.loading_images);

			for (const auto& input_img_id : subjects.images) {
				auto& out_entry = baked_images[input_img_id];

				try {
					const auto it = loaded_images.try_emplace(input_img_id, input_img_id);

					{
						const bool is_img_unique = it.second;
						ensure(is_img_unique);
					}

					auto& img = (*it.first).second;

#if DEBUG_FILL_IMGS_WITH_COLOR
					img.fill(rgba().set_hsv({ rng.randval(0.0f, 1.0f), rng.randval(0.3f, 1.0f), rng.randval(0.3f, 1.0f) }));
#endif

					const auto u_size = img.get_size();
					out_entry.cached_original_size_pixels = u_size;

					const auto size = vec2i(u_size);
					rects_for_packer.push_back(rect_xywh(0, 0, size.x, size.y));
				}
				catch (augs::image_loading_error err) {
					out_entry.cached_original_size_pixels = vec2u::zero;
				}
			}
		}

		{
			auto scope = measure_scope(in.profiler.loading_fonts);

			for (const auto& input_fnt_id : subjects.fonts) {
				const auto it = loaded_fonts.try_emplace(input_fnt_id, input_fnt_id);

				{
					const bool is_font_unique = it.second;
					ensure(is_font_unique);
				}

				const auto& fnt = (*it.first).second;

				auto& out_fnt = stored_baked_fonts[input_fnt_id];
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
					img.fill(rgba().set_hsv({ rng.randval(0.0f, 1.0f), rng.randval(0.3f, 1.0f), rng.randval(0.3f, 1.0f) }));
				}
#endif
			}
		}

		{
			auto scope = measure_scope(in.profiler.packing);

			const auto max_size = static_cast<int>(settings.packer_detail_max_atlas_size);
			const auto rect_padding_amount = 0;

			in.profiler.subjects_count.measure(rects_for_packer.size());

			for (auto& rr : rects_for_packer) {
				rr.w += rect_padding_amount;
				rr.h += rect_padding_amount;
			}

			constexpr bool allow_flip = true;

			using spaces_type = rectpack2D::empty_spaces<allow_flip>;
			using rect_ptr = rect_xywhf*;

			auto pathology_sort = [](const rect_ptr a, const rect_ptr b) {
				return a->get_wh().pathological_mult() > b->get_wh().pathological_mult();
			};

#if 1
			const auto result_size = find_best_packing<spaces_type>(
				rects_for_packer,
				make_finder_input(
					max_size,
					1,
					[](auto){ return true; },
					[](auto){ ensure(false); return false; }
				)
			);
			(void)pathology_sort;

#else
			std::vector<rect_ptr> input_for_packer;

			for (auto& r : rects_for_packer) {
				input_for_packer.push_back(&r);
			}

			(void)max_size;
			(void)pathology_sort;
			auto packing_root = spaces_type({ 2432, 2432 });

			sort_range(input_for_packer,[](const rect_ptr a, const rect_ptr b) {
				//return std::max(a->w, a->h) > std::max(b->w, b->h);
				return a->get_wh().pathological_mult() > b->get_wh().pathological_mult();
				});


			for (auto* rr : input_for_packer) {
				if (const auto n = packing_root.insert(rr->get_wh())) {
					*rr = *n;
				}
				else {
					ensure(false);
					break;
				}
			}

			const auto result_size = packing_root.get_rects_aabb();

#endif
			atlas_image_size = vec2u(result_size.w, result_size.h);

			std::size_t total_used_space = 0;

			for (auto& rr : rects_for_packer) {
				total_used_space += rr.area();
			}

			for (auto& rr : rects_for_packer) {
				rr.w -= rect_padding_amount;
				rr.h -= rect_padding_amount;
			}

			in.profiler.atlas_size.measure(atlas_image_size);
			const auto wasted_space = atlas_image_size.area() - total_used_space;
			in.profiler.wasted_space.measure(wasted_space);
			in.profiler.wasted_space_percent.measure(100 * double(wasted_space) / atlas_image_size.area());
			//in.profiler.atlas_width.measure(atlas_image_size.x);
		}

		// translate pixels into atlas space and render the image

		{
			auto scope = measure_scope(in.profiler.resizing_image);

			output_image.resize_no_fill(atlas_image_size);

#if DEBUG_FILL_IMGS_WITH_COLOR
			output_image.fill({0, 0, 0, 255});
#else

#endif
		}
		
		size_t current_rect = 0u;

		{
			auto scope = measure_scope(in.profiler.blitting_images);

			for (const auto& input_img_id : subjects.images) {
				const auto packed_rect = rects_for_packer[current_rect];

				{
					auto& output_entry = baked_images[input_img_id];

					if (output_entry.cached_original_size_pixels.non_zero()) {
						output_entry.atlas_space.set(
							static_cast<float>(packed_rect.x) / atlas_image_size.x,
							static_cast<float>(packed_rect.y) / atlas_image_size.y,
							static_cast<float>(packed_rect.w) / atlas_image_size.x,
							static_cast<float>(packed_rect.h) / atlas_image_size.y
						);

						output_entry.was_flipped = packed_rect.flipped;
					}
					else {
						output_entry.atlas_space.set(0.f, 0.f, 1.f, 1.f);
						output_entry.cached_original_size_pixels = atlas_image_size;
						output_entry.was_flipped = false;

						continue;
					}
				}

				output_image.blit(
					loaded_images[input_img_id],
					{
						static_cast<unsigned>(packed_rect.x),
						static_cast<unsigned>(packed_rect.y)
					},
					packed_rect.flipped
				);

				++current_rect;
			}
		}

		{
			auto scope = measure_scope(in.profiler.blitting_fonts);

			for (auto& input_font_id : subjects.fonts) {
				auto& output_font = stored_baked_fonts[input_font_id];

				for (size_t glyph_index = 0; glyph_index < output_font.glyphs_in_atlas.size(); ++glyph_index) {
					const auto& packed_rect = rects_for_packer[current_rect];

					auto& g = output_font.glyphs_in_atlas[glyph_index];

					g.atlas_space.set(
						static_cast<float>(packed_rect.x) / atlas_image_size.x,
						static_cast<float>(packed_rect.y) / atlas_image_size.y,
						static_cast<float>(packed_rect.w) / atlas_image_size.x,
						static_cast<float>(packed_rect.h) / atlas_image_size.y
					);

					g.was_flipped = packed_rect.flipped;

					output_image.blit(
						loaded_fonts.at(input_font_id).glyph_bitmaps[glyph_index],
						{
							static_cast<unsigned>(packed_rect.x),
							static_cast<unsigned>(packed_rect.y)
						},
						packed_rect.flipped
					);

					++current_rect;
				}
			}
		}

		auto scope = measure_scope(in.profiler.saving);

		output_image.save(atlas_image_path);
		augs::save_as_bytes(new_stamp, atlas_stamp_path);
		augs::save_as_bytes(*this, atlas_metadata_path);
	}
	else {
		output_image.from_file(atlas_image_path);
		augs::load_from_bytes(*this, atlas_metadata_path);
	}
}