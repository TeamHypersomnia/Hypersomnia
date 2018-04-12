#include <string>
#include <sstream>

#include "3rdparty/rectpack2D/src/pack.h"

#include "augs/ensure.h"

#include "augs/readwrite/memory_stream.h"

#include "augs/image/image.h"
#include "augs/texture_atlas/texture_atlas.h"

#include "augs/readwrite/byte_file.h"
#include "augs/filesystem/directory.h"

regenerated_atlas::regenerated_atlas(
	const atlas_regeneration_input& in,
	const atlas_regeneration_settings settings,
	augs::image& output_image
) {
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
		for (const auto& img_id : in.images) {
			new_stamp.image_stamps[img_id] = augs::last_write_time(img_id);
		}

		for (const auto& fnt_id : in.fonts) {
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

		for (const auto& input_img_id : in.images) {
			const auto it = loaded_images.try_emplace(input_img_id, input_img_id);
			
			{
				const bool is_img_unique = it.second;
				ensure(is_img_unique);
			}
			
			const auto& img = (*it.first).second;

			auto& out_entry = baked_images[input_img_id];
			out_entry.cached_original_size_pixels = img.get_size();

			rects_for_packing_algorithm.push_back({
				0,
				0,
				static_cast<int>(img.get_size().x),
				static_cast<int>(img.get_size().y)
			});
		}

		for (const auto& input_fnt_id : in.fonts) {
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

		std::vector<rect_xywhf*> input_for_packing_algorithm;

		for (auto& r : rects_for_packing_algorithm) {
			input_for_packing_algorithm.push_back(&r);
		}

		std::vector<bin> packing_output;

		const auto rect_padding_amount = 2;

		for (auto& rr : rects_for_packing_algorithm) {
			rr.w += rect_padding_amount;
			rr.h += rect_padding_amount;
		}

		const bool result = pack(
			input_for_packing_algorithm.data(), 
			static_cast<int>(input_for_packing_algorithm.size()), 
			static_cast<int>(settings.packer_detail_max_atlas_size),
			true,
			packing_output
		);

		for (auto& rr : rects_for_packing_algorithm) {
			rr.w -= rect_padding_amount;
			rr.h -= rect_padding_amount;
		}

		const bool textures_dont_fit_into_atlas = !result || packing_output.size() > 1;

		ensure(!textures_dont_fit_into_atlas);
		ensure_eq(packing_output[0].rects.size(), input_for_packing_algorithm.size());

		atlas_image_size = {
			static_cast<unsigned>(packing_output[0].size.w),
			static_cast<unsigned>(packing_output[0].size.h)
		};

		// translate pixels into atlas space and render the image

		output_image.resize(atlas_image_size);
		
		size_t current_rect = 0u;

		for (auto& input_img_id : in.images) {
			auto& output_img = baked_images[input_img_id];

			const auto packed_rect = rects_for_packing_algorithm[current_rect];

			output_img.atlas_space.set(
				static_cast<float>(packed_rect.x) / atlas_image_size.x,
				static_cast<float>(packed_rect.y) / atlas_image_size.y,
				static_cast<float>(packed_rect.w) / atlas_image_size.x,
				static_cast<float>(packed_rect.h) / atlas_image_size.y
			);

			output_img.was_flipped = packed_rect.flipped;

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

		for (auto& input_font_id : in.fonts) {
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

		output_image.save(atlas_image_path);
		augs::save_as_bytes(new_stamp, atlas_stamp_path);
		augs::save_as_bytes(*this, atlas_metadata_path);
	}
	else {
		output_image.from_file(atlas_image_path);
		augs::load_from_bytes(*this, atlas_metadata_path);
	}
}