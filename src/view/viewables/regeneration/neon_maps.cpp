#include <sstream>

#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "augs/misc/simple_pair.h"

#include "augs/ensure.h"
#include "augs/readwrite/memory_stream.h"

#include "augs/image/image.h"

#include "view/viewables/regeneration/neon_maps.h"

#include "augs/readwrite/byte_file.h"

#define PIXEL_NONE rgba(0,0,0,0)

void make_neon(
	const neon_map_input& input,
	augs::image& source
);

std::vector<std::vector<double>> generate_gauss_kernel(
	const neon_map_input& input
);

std::vector<vec2u> hide_undesired_pixels(
	augs::image& original_image,
	const std::vector<rgba>& color_whitelist
);

void resize_image(
	augs::image& image_to_resize,
	const vec2u size
);

void cut_empty_edges(augs::image& source);

void regenerate_neon_map(
	const augs::path_type& input_image_path,
	const augs::path_type& output_image_path,
	const neon_map_input in,
	const bool force_regenerate
) {
	neon_map_stamp new_stamp;
	new_stamp.input = in;
	new_stamp.last_write_time_of_source = augs::last_write_time(input_image_path);

	const auto neon_map_path = output_image_path;
	const auto neon_map_stamp_path = augs::path_type(neon_map_path).replace_extension(".stamp");

	const auto new_stamp_bytes = augs::to_bytes(new_stamp);

	bool should_regenerate = force_regenerate;

	if (!augs::exists(neon_map_path)) {
		should_regenerate = true;
	}
	else {
		if (!augs::exists(neon_map_stamp_path)) {
			should_regenerate = true;
		}
		else {
			const auto existent_stamp_bytes = augs::file_to_bytes(neon_map_stamp_path);
			const bool are_stamps_identical = (new_stamp_bytes == existent_stamp_bytes);

			if (!are_stamps_identical) {
				should_regenerate = true;
			}
		}
	}

	if (should_regenerate) {
		LOG("Regenerating neon map for %x", input_image_path);

		auto source_image = augs::image(input_image_path);
		make_neon(new_stamp.input, source_image);

		source_image.save(neon_map_path);

		augs::create_directories_for(neon_map_stamp_path);
		augs::save_as_bytes(new_stamp_bytes, neon_map_stamp_path);
	}
}

void make_neon(
	const neon_map_input& input,
	augs::image& source
) {
	resize_image(source, vec2u(input.radius_towards_x_axis, input.radius_towards_y_axis));

	const auto pixel_list = hide_undesired_pixels(source, input.light_colors);

	const auto kernel = generate_gauss_kernel(input);

	std::vector<rgba> pixel_colors;

	for (const auto& pixel : pixel_list) {
		pixel_colors.push_back(source.pixel({ pixel.x, pixel.y }));
	}

	unsigned i = 0;

	for (auto& pixel : pixel_list) {
		rgba center_pixel = pixel_colors[i];
		++i;

		const auto center_pixel_rgba = rgba{ center_pixel[2], center_pixel[1], center_pixel[0], center_pixel[3] };

		for (unsigned y = 0; y < kernel.size(); ++y) {
			for (unsigned x = 0; x < kernel[y].size(); ++x) {
				unsigned current_index_y = pixel.y + y - input.radius_towards_y_axis / 2;

				if (current_index_y >= source.get_rows()) {
					continue;
				}

				unsigned current_index_x = pixel.x + x - input.radius_towards_x_axis / 2;

				if (current_index_x >= source.get_columns()) {
					continue;
				}

				rgba& current_pixel = source.pixel({ current_index_x, current_index_y });
				auto current_pixel_rgba = rgba{ current_pixel[2], current_pixel[1], current_pixel[0], current_pixel[3] };
				auto alpha = static_cast<unsigned>(255 * kernel[y][x] * input.amplification);
				alpha = alpha > 255 ? 255 : alpha;

				if (current_pixel_rgba == PIXEL_NONE && alpha) {
					current_pixel[2] = center_pixel[2];
					current_pixel[1] = center_pixel[1];
					current_pixel[0] = center_pixel[0];
				}

				else if (current_pixel_rgba != center_pixel_rgba && alpha) {
					current_pixel[2] = static_cast<rgba_channel>((alpha * center_pixel[2] + current_pixel[3] * current_pixel[2]) / (alpha + current_pixel[3]));
					current_pixel[1] = static_cast<rgba_channel>((alpha * center_pixel[1] + current_pixel[3] * current_pixel[1]) / (alpha + current_pixel[3]));
					current_pixel[0] = static_cast<rgba_channel>((alpha * center_pixel[0] + current_pixel[3] * current_pixel[0]) / (alpha + current_pixel[3]));
				}

				if (alpha > current_pixel[3]) {
					current_pixel[3] = static_cast<rgba_channel>(alpha);
				}
			}
		}
	}

	for (std::size_t i = 0; i < pixel_list.size(); ++i) {
		source.pixel(pixel_list[i]) = pixel_colors[i];
	}

	cut_empty_edges(source);

	for (unsigned y = 0; y < source.get_rows(); ++y) {
		for (unsigned x = 0; x < source.get_columns(); ++x) {
			source.pixel({ x, y }).a = static_cast<rgba_channel>(source.pixel({ x, y }).a * input.alpha_multiplier);
		}
	}
}

std::vector<std::vector<double>> generate_gauss_kernel(const neon_map_input& input)
{
	const auto radius_towards_x_axis = input.radius_towards_x_axis;
	auto radius_towards_y_axis = input.radius_towards_y_axis;

	if (!input.radius_towards_y_axis) {
		radius_towards_y_axis = radius_towards_x_axis;
	}

	std::vector<std::vector<augs::simple_pair<int, int>>> index;

	auto max_index_x = radius_towards_x_axis / 2;
	auto max_index_y = radius_towards_y_axis / 2;

	index.resize(radius_towards_y_axis);

	for (auto& vector : index) {
		vector.resize(radius_towards_x_axis);
	}

	for (unsigned y = 0; y < index.size(); ++y) {
		for (unsigned x = 0; x < index[y].size(); ++x) {
			index[y][x] = { static_cast<int>(x - max_index_x), static_cast<int>(y - max_index_y) };
		}
	}

	std::vector<std::vector<double>> result;
	result.resize(radius_towards_y_axis);

	for (auto& vector : result) {
		vector.resize(radius_towards_x_axis);
	}

	for (unsigned y = 0; y < result.size(); ++y) {
		for (unsigned x = 0; x < result[y].size(); ++x) {
			result[y][x] = exp(-1 * (pow(index[x][y].first, 2) + pow(index[x][y].second, 2)) / 2 / pow(input.standard_deviation, 2)) / PI<float> / 2 / pow(input.standard_deviation, 2);
		}
	}

	double sum = 0.f;

	for (const auto& vec_2d : result) {
		for (const auto& value : vec_2d) {
			sum += value;
		}
	}

	for (auto& vector : result) {
		for (auto& value : vector) {
			value /= sum;
		}
	}

	return result;
}


void resize_image(
	augs::image& image_to_resize,
	vec2u size
) {

	size.x = image_to_resize.get_columns() + size.x * 2;
	size.y = image_to_resize.get_rows() + size.y * 2;

	auto copy_mat = augs::image(size);

	auto offset_x = static_cast<int>(size.x - image_to_resize.get_columns()) / 2;

	if (offset_x < 0) {
		offset_x = 0;
	}

	auto offset_y = static_cast<int>(size.y - image_to_resize.get_rows()) / 2;

	if (offset_y < 0) {
		offset_y = 0;
	}

	for (unsigned y = 0; y < image_to_resize.get_rows(); ++y) {
		for (unsigned x = 0; x < image_to_resize.get_columns(); ++x)
		{
			copy_mat.pixel({ x + offset_x, y + offset_y }) = image_to_resize.pixel({ x, y });;
		}
	}

	image_to_resize = copy_mat;
}

std::vector<vec2u> hide_undesired_pixels(
	augs::image& original_image,
	const std::vector<rgba>& color_whitelist
) {
	std::vector<vec2u> result;

	for (unsigned y = 0; y < original_image.get_rows(); ++y) {
		for (unsigned x = 0; x < original_image.get_columns(); ++x)
		{
			auto& pixel = original_image.pixel({ x, y });
			auto found = find_if(color_whitelist.begin(), color_whitelist.end(), [pixel](const rgba& a) {
				return a == pixel;
			});

			if (found == color_whitelist.end())
			{
				pixel = PIXEL_NONE;
			}
			else {
				result.push_back(vec2u{ x, y });
			}
		}
	}

	return result;
}

void cut_empty_edges(augs::image& source) {
	vec2u output_size = source.get_size();
	vec2u offset = { 0,0 };
	for (unsigned y = 0; y < source.get_rows() / 2; ++y) {
		bool pixel_found = false;
		for (unsigned x = 0; x < source.get_columns(); ++x) {
			if (source.pixel({ x, y }) != PIXEL_NONE || source.pixel({ x, source.get_rows() - y - 1 }) != PIXEL_NONE) {
				pixel_found = true;
				break;
			}
		}
		if (!pixel_found) {
			output_size.y -= 2;
			++offset.y;
		}
		else
			break;
	}

	for (unsigned x = 0; x < source.get_columns() / 2; ++x) {
		bool pixel_found = false;
		for (unsigned y = 0; y < source.get_rows(); ++y) {
			if (source.pixel({ x, y }) != PIXEL_NONE || source.pixel({ source.get_columns() - x - 1, y }) != PIXEL_NONE) {
				pixel_found = true;
				break;
			}
		}
		if (!pixel_found) {
			output_size.x -= 2;
			++offset.x;
		}
		else
			break;
	}

	if (offset == vec2u(0, 0) || output_size.x == 0 || output_size.y == 0) {
		return;
	}

	auto copy = augs::image(output_size);

	for (unsigned x = 0; x < copy.get_columns(); ++x) {
		for (unsigned y = 0; y < copy.get_rows(); ++y) {
			copy.pixel({ x,y }) = source.pixel({ x + offset.x, y + offset.y });
		}
	}

	source = copy;
}