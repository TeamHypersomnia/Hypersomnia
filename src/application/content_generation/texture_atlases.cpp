#include <unordered_map>
#include <map>
#include <string>
#include "texture_atlases.h"

#include <sstream>

#include "game/assets/assets_manager.h"
#include "augs/filesystem/directory.h"
#include "augs/filesystem/file.h"

#include "augs/ensure.h"
#include "augs/misc/streams.h"

#include "augs/image/image.h"

#include "augs/graphics/renderer.h"
#include "3rdparty/rectpack2D/src/pack.h"

#include "augs/misc/templated_readwrite.h"
#include "generated/introspectors.h"

atlases_regeneration_output regenerate_atlases(
	const atlases_regeneration_input& in,
	const bool force_regenerate,
	const bool always_check_source_images_integrity,
	const bool save_atlases_as_binary,
	const unsigned packer_detail_max_atlas_size
) {
	atlases_regeneration_output output;

	struct per_atlas_input {
		std::vector<source_font_identifier> fonts;
		std::vector<source_image_identifier> images;
	};

	std::unordered_map<
		assets::gl_texture_id,
		per_atlas_input
	> per_atlas_inputs;

	for (const auto& i : in.images) {
		per_atlas_inputs[i.target_atlas].images.push_back(i.path);
	}

	for (const auto& i : in.fonts) {
		per_atlas_inputs[i.target_atlas].fonts.push_back(i.loading_input);
	}

	const auto& manager = get_assets_manager();

	const std::string atlases_directory = "generated/atlases/";

	for (const auto& input_for_this_atlas : per_atlas_inputs) {
		const auto atlas_stem = typesafe_sprintf("%x", static_cast<int>(input_for_this_atlas.first));
		const auto atlas_image_path = atlases_directory + atlas_stem + (save_atlases_as_binary ? ".bin" : ".png");
		const auto atlas_metadata_path = atlases_directory + atlas_stem + ".meta";
		const auto atlas_stamp_path = atlases_directory + atlas_stem + ".stamp";

		bool should_regenerate = force_regenerate;

		texture_atlas_stamp new_stamp;

		/* This check is optional to improve launch times */

		if (always_check_source_images_integrity) {
			for (const auto& img_id : input_for_this_atlas.second.images) {
				new_stamp.image_stamps[img_id] = augs::last_write_time(img_id);
			}

			for (const auto& fnt_id : input_for_this_atlas.second.fonts) {
				new_stamp.font_stamps[fnt_id] = augs::last_write_time(fnt_id.path);
			}

			if (!augs::file_exists(atlas_image_path)) {
				should_regenerate = true;
			}
			else {
				if (!augs::file_exists(atlas_stamp_path)) {
					should_regenerate = true;
				}
				else {
					augs::stream existent_stamp_stream;

					augs::get_file_contents_binary_into(atlas_stamp_path, existent_stamp_stream);
					texture_atlas_stamp existent_stamp;
					augs::read(existent_stamp_stream, existent_stamp);

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

			texture_atlas_metadata this_atlas_metadata;

			std::unordered_map<source_image_identifier, augs::image> loaded_images;
			std::unordered_map<source_font_identifier, augs::font> loaded_fonts;

			std::vector<rect_xywhf> rects_for_packing_algorithm;

			for (const auto& input_img_id : input_for_this_atlas.second.images) {
				auto& img = loaded_images[input_img_id];
				img.from_file(input_img_id);

				auto& out_entry = this_atlas_metadata.images[input_img_id];
				out_entry.original_size_pixels = img.get_size();

				rects_for_packing_algorithm.push_back(
					rect_xywhf(
						0,
						0,
						static_cast<int>(img.get_size().x),
						static_cast<int>(img.get_size().y)
					)
				);
			}

			for (const auto& input_fnt_id : input_for_this_atlas.second.fonts) {
				auto& fnt = loaded_fonts[input_fnt_id];
				fnt.from_file(input_fnt_id.path, input_fnt_id.input);
				
				auto& out_fnt = this_atlas_metadata.fonts[input_fnt_id];
				out_fnt.meta_from_file = fnt.meta;

				for (const auto& g : fnt.glyph_bitmaps) {
					out_fnt.glyphs_in_atlas.push_back({});

					auto& out_entry = *out_fnt.glyphs_in_atlas.rbegin();

					out_entry.original_size_pixels = g.get_size();

					rects_for_packing_algorithm.push_back(
						rect_xywhf(
							0,
							0,
							static_cast<int>(g.get_size().x),
							static_cast<int>(g.get_size().y)
						)
					);
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
				static_cast<int>(packer_detail_max_atlas_size),
				packing_output
			);

			for (auto& rr : rects_for_packing_algorithm) {
				rr.w -= rect_padding_amount;
				rr.h -= rect_padding_amount;
			}

			const bool textures_dont_fit_into_atlas = !result || packing_output.size() > 1;

			ensure(!textures_dont_fit_into_atlas);
			ensure(packing_output[0].rects.size() == input_for_packing_algorithm.size());

			const auto atlas_size = vec2u(
				static_cast<unsigned>(packing_output[0].size.w),
				static_cast<unsigned>(packing_output[0].size.h)
			);

			this_atlas_metadata.atlas_image_size = atlas_size;

			// translate pixels into atlas space and render the image

			augs::image atlas_image;
			atlas_image.create(atlas_size);
			
			size_t current_rect = 0u;

			for (auto& input_img_id : input_for_this_atlas.second.images) {
				auto& output_img = this_atlas_metadata.images[input_img_id];

				const auto& packed_rect = rects_for_packing_algorithm[current_rect];

				output_img.atlas_space.set(
					static_cast<float>(packed_rect.x) / atlas_size.x,
					static_cast<float>(packed_rect.y) / atlas_size.y,
					static_cast<float>(packed_rect.w) / atlas_size.x,
					static_cast<float>(packed_rect.h) / atlas_size.y
				);

				output_img.was_flipped = packed_rect.flipped;

				atlas_image.blit(
					loaded_images[input_img_id],
					{
						static_cast<unsigned>(packed_rect.x),
						static_cast<unsigned>(packed_rect.y)
					},
					packed_rect.flipped
				);

				++current_rect;
			}

			for (auto& input_font_id : input_for_this_atlas.second.fonts) {
				auto& output_font = this_atlas_metadata.fonts[input_font_id];

				for (size_t glyph_index = 0; glyph_index < output_font.glyphs_in_atlas.size(); ++glyph_index) {
					const auto& packed_rect = rects_for_packing_algorithm[current_rect];

					auto& g = output_font.glyphs_in_atlas[glyph_index];

					g.atlas_space.set(
						static_cast<float>(packed_rect.x) / atlas_size.x,
						static_cast<float>(packed_rect.y) / atlas_size.y,
						static_cast<float>(packed_rect.w) / atlas_size.x,
						static_cast<float>(packed_rect.h) / atlas_size.y
					);

					g.was_flipped = packed_rect.flipped;

					atlas_image.blit(
						loaded_fonts[input_font_id].glyph_bitmaps[glyph_index],
						{
							static_cast<unsigned>(packed_rect.x),
							static_cast<unsigned>(packed_rect.y)
						},
						packed_rect.flipped
					);

					++current_rect;
				}
			}

			atlas_image.swap_red_and_blue();

			if (save_atlases_as_binary) {
				atlas_image.save_as_binary_file(atlas_image_path);
			}
			else {
				atlas_image.save(atlas_image_path);
			}

			{
				augs::stream new_stamp_stream;
				augs::write(new_stamp_stream, new_stamp);

				augs::create_binary_file(atlas_stamp_path, new_stamp_stream);
			}

			{
				augs::stream new_meta_stream;
				augs::write(new_meta_stream, this_atlas_metadata);

				augs::create_binary_file(atlas_metadata_path, new_meta_stream);
			}

			output.metadatas.emplace_back(std::move(std::make_pair(input_for_this_atlas.first, this_atlas_metadata)));
		}
		else {
			texture_atlas_metadata this_atlas_metadata;

			augs::stream existent_meta_stream;
			augs::get_file_contents_binary_into(atlas_metadata_path, existent_meta_stream);

			augs::read(existent_meta_stream, this_atlas_metadata);

			output.metadatas.emplace_back(std::move(std::make_pair(
				input_for_this_atlas.first, 
				this_atlas_metadata
			)));
		}
	}

	return output;
}