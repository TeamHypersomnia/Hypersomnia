#include <string>
#include <sstream>

#include "3rdparty/rectpack2D/src/pack.h"

#include "augs/ensure.h"
#include "augs/misc/measurements.h"

#include "augs/readwrite/memory_stream.h"

#include "augs/image/image.h"
#include "augs/texture_atlas/texture_atlas.h"

#include "augs/readwrite/byte_file.h"
#include "augs/filesystem/directory.h"

using namespace rectpack;

regenerated_atlas::regenerated_atlas(regenerated_atlas_input in) {
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

		std::vector<rect_xywhf> rects_for_packing_algorithm;

		{
			auto scope = measure_scope(in.profiler.images.loading);

			for (const auto& input_img_id : subjects.images) {
				auto& out_entry = baked_images[input_img_id];

				try {
					const auto it = loaded_images.try_emplace(input_img_id, input_img_id);

					{
						const bool is_img_unique = it.second;
						ensure(is_img_unique);
					}

					const auto& img = (*it.first).second;

					const auto u_size = img.get_size();
					out_entry.cached_original_size_pixels = u_size;

					const auto size = vec2i(u_size);
					rects_for_packing_algorithm.push_back({0, 0, size.x, size.y});
				}
				catch (augs::image_loading_error err) {
					out_entry.cached_original_size_pixels = vec2u::zero;
				}
			}
		}

		{
			auto scope = measure_scope(in.profiler.fonts.loading);

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

					rects_for_packing_algorithm.push_back({
						0,
						0,
						static_cast<int>(g.get_size().x),
						static_cast<int>(g.get_size().y)
					});
				}
			}
		}

		{
			auto scope = measure_scope(in.profiler.packing);

			const auto max_size = static_cast<int>(settings.packer_detail_max_atlas_size);
			const auto rect_padding_amount = 2;

#if 0
			std::vector<rect_xywhf*> input_for_packing_algorithm;

			for (auto& r : rects_for_packing_algorithm) {
				input_for_packing_algorithm.push_back(&r);
			}

			for (auto& rr : rects_for_packing_algorithm) {
				rr.w += rect_padding_amount;
				rr.h += rect_padding_amount;
			}

			const auto result_size = pack_rectangles(
				input_for_packing_algorithm,
				max_size,
				true,
				[](auto){},
				[](auto){},
				128
			);

			for (auto& rr : rects_for_packing_algorithm) {
				rr.w -= rect_padding_amount;
				rr.h -= rect_padding_amount;
			}

			atlas_image_size.set(result_size.w, result_size.h);
#else
			auto packing_root = rectpack::node::make_root({ max_size, max_size });

			vec2i result_size;
			
			for (auto& rr : rects_for_packing_algorithm) {
				rr.w += rect_padding_amount;
				rr.h += rect_padding_amount;

				if (const auto n = packing_root.insert(rr, true)) {
					n->readback(rr, result_size);

					rr.w -= rect_padding_amount;
					rr.h -= rect_padding_amount;
				}
				else {
					ensure(false);
					break;
				}
			}

			atlas_image_size = static_cast<vec2u>(result_size);
#endif

		}

		// translate pixels into atlas space and render the image

		{
			auto scope = measure_scope(in.profiler.resizing_image);

			output_image.resize(atlas_image_size);
		}
		
		size_t current_rect = 0u;

		{
			auto scope = measure_scope(in.profiler.images.blitting);

			for (const auto& input_img_id : subjects.images) {
				const auto packed_rect = rects_for_packing_algorithm[current_rect];

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
			auto scope = measure_scope(in.profiler.fonts.blitting);

			for (auto& input_font_id : subjects.fonts) {
				auto& output_font = stored_baked_fonts[input_font_id];

				for (size_t glyph_index = 0; glyph_index < output_font.glyphs_in_atlas.size(); ++glyph_index) {
					const auto& packed_rect = rects_for_packing_algorithm[current_rect];

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