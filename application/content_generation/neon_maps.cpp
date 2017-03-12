#include <sstream>
#include <experimental/filesystem>

#include "neon_maps.h"
#include "augs/filesystem/directory.h"
#include "augs/filesystem/file.h"

#include "augs/ensure.h"
#include "augs/misc/streams.h"

#include "augs/image/image.h"
#include "generated_introspectors.h"

#define PIXEL_NONE rgba(0,0,0,0)

namespace fs = std::experimental::filesystem;

void make_neon(
	const neon_map_stamp& stamp,
	augs::image& source
);

std::vector<std::vector<double>> generate_gauss_kernel(
	const neon_map_stamp& stamp
);

std::vector<vec2u> hide_undesired_pixels(
	augs::image& original_image,
	const std::vector<rgba>& color_whitelist);

void resize_image(
	augs::image& image_to_resize,
	const vec2u size
);

void cut_empty_edges(augs::image& source);

void regenerate_neon_maps(
	const bool force_regenerate
) {
	const auto neon_directory = "generated/neon_maps/";

	augs::create_directories(neon_directory);

	const auto lines = augs::get_file_lines("neon_map_generator_input.cfg");
	size_t current_line = 0;

	while (current_line < lines.size()) {
		neon_map_stamp new_stamp;

		const auto source_path = fs::path(lines[current_line]);

		new_stamp.last_write_time_of_source = augs::last_write_time(source_path.string());

		ensure(lines[current_line + 1] == "whitelist:");

		current_line += 2;

		while (lines[current_line] != "parameters:") {
			std::istringstream in(lines[current_line]);

			rgba pixel;
			in >> pixel;

			new_stamp.light_colors.push_back(pixel);

			++current_line;
		}

		// skip "parameters:" line
		++current_line;

		std::istringstream in(lines[current_line]);

		in
			>> new_stamp.standard_deviation
			>> new_stamp.radius_towards_x_axis
			>> new_stamp.radius_towards_y_axis
			>> new_stamp.amplification
			>> new_stamp.alpha_multiplier
		;

		const auto neon_map_path = neon_directory + source_path.filename().string();
		const auto neon_map_stamp_path = neon_directory + source_path.filename().replace_extension(".stamp").string();

		augs::stream new_stamp_stream;
		augs::write(new_stamp_stream, new_stamp);

		bool should_regenerate = force_regenerate;

		if (!augs::file_exists(neon_map_path)) {
			should_regenerate = true;
		}
		else {
			if (!augs::file_exists(neon_map_stamp_path)) {
				should_regenerate = true;
			}
			else {
				augs::stream existent_stamp_stream;
				augs::assign_file_contents_binary(neon_map_stamp_path, existent_stamp_stream);

				const bool are_stamps_identical = (new_stamp_stream == existent_stamp_stream);

				if (!are_stamps_identical) {
					should_regenerate = true;
				}
			}
		}

		if (should_regenerate) {
			LOG("Regenerating neon map for %x", source_path.string());

			augs::image source_image;
			source_image.from_file(source_path.string());

			make_neon(new_stamp, source_image);

			source_image.save(neon_map_path);

			augs::create_binary_file(neon_map_stamp_path, new_stamp_stream);
		}

		// skip parameters line
		++current_line;

		// skip separating newline
		++current_line;
	}
}

void make_neon(
	const neon_map_stamp& stamp,
	augs::image& source
) {

	resize_image(source, vec2u(stamp.radius_towards_x_axis, stamp.radius_towards_y_axis));

	const auto pixel_list = hide_undesired_pixels(source, stamp.light_colors);

	const auto kernel = generate_gauss_kernel(stamp);

	std::vector<rgba> pixel_colors;

	for (const auto& pixel : pixel_list) {
		pixel_colors.push_back(source.pixel({ pixel.x, pixel.y }));
	}

	size_t i = 0;

	for (auto& pixel : pixel_list) {
		rgba center_pixel = pixel_colors[i];
		++i;

		const auto center_pixel_rgba = rgba{ center_pixel[2], center_pixel[1], center_pixel[0], center_pixel[3] };

		for (size_t y = 0; y < kernel.size(); ++y) {
			for (size_t x = 0; x < kernel[y].size(); ++x) {
				size_t current_index_y = pixel.y + y - stamp.radius_towards_y_axis / 2;

				if (current_index_y < 0 || current_index_y >= source.get_rows()) {
					continue;
				}

				size_t current_index_x = pixel.x + x - stamp.radius_towards_x_axis / 2;

				if (current_index_x < 0 || current_index_x >= source.get_columns()) {
					continue;
				}

				rgba& current_pixel = source.pixel({ current_index_x, current_index_y });
				auto current_pixel_rgba = rgba{ current_pixel[2], current_pixel[1], current_pixel[0], current_pixel[3] };
				size_t alpha = static_cast<size_t>(255 * kernel[y][x] * stamp.amplification);
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

	for (size_t i = 0; i < pixel_list.size(); ++i) {
		source.pixel(pixel_list[i]) = pixel_colors[i];
	}

	cut_empty_edges(source);

	for (size_t y = 0; y < source.get_rows(); ++y) {
		for (size_t x = 0; x < source.get_columns(); ++x) {
			source.pixel({ x, y }).a = static_cast<rgba_channel>(source.pixel({ x, y }).a * stamp.alpha_multiplier);
		}
	}
}

std::vector<std::vector<double>> generate_gauss_kernel(const neon_map_stamp& stamp)
{
	const auto radius_towards_x_axis = stamp.radius_towards_x_axis;
	const auto radius_towards_y_axis = stamp.radius_towards_y_axis;

	if (!stamp.radius_towards_y_axis) {
		const auto radius_towards_y_axis = radius_towards_x_axis;
	}

	std::vector<std::vector<std::pair<int, int> > > index;

	auto max_index_x = radius_towards_x_axis / 2;
	auto max_index_y = radius_towards_y_axis / 2;

	index.resize(radius_towards_y_axis);

	for (auto& vector : index) {
		vector.resize(radius_towards_x_axis);
	}

	for (size_t y = 0; y < index.size(); ++y) {
		for (size_t x = 0; x < index[y].size(); ++x) {
			index[y][x] = std::make_pair(x - max_index_x, y - max_index_y);
		}
	}

	std::vector<std::vector<double>> result;
	result.resize(radius_towards_y_axis);

	for (auto& vector : result) {
		vector.resize(radius_towards_x_axis);
	}

	for (size_t y = 0; y < result.size(); ++y) {
		for (size_t x = 0; x < result[y].size(); ++x) {
			result[y][x] = exp(-1 * (pow(index[x][y].first, 2) + pow(index[x][y].second, 2)) / 2 / pow(stamp.standard_deviation, 2)) / PI_f / 2 / pow(stamp.standard_deviation, 2);
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

	augs::image copy_mat;
	copy_mat.create(size);

	auto offset_x = static_cast<int>(size.x - image_to_resize.get_columns()) / 2;

	if (offset_x < 0) {
		offset_x = 0;
	}

	auto offset_y = static_cast<int>(size.y - image_to_resize.get_rows()) / 2;

	if (offset_y < 0) {
		offset_y = 0;
	}

	for (size_t y = 0; y < image_to_resize.get_rows(); ++y) {
		for (size_t x = 0; x < image_to_resize.get_columns(); ++x)
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

	for (size_t y = 0; y < original_image.get_rows(); ++y) {
		for (size_t x = 0; x < original_image.get_columns(); ++x)
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
	for (size_t y = 0; y < source.get_rows() / 2; ++y) {
		bool pixel_found = false;
		for (size_t x = 0; x < source.get_columns(); ++x) {
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

	for (size_t x = 0; x < source.get_columns() / 2; ++x) {
		bool pixel_found = false;
		for (size_t y = 0; y < source.get_rows(); ++y) {
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

	augs::image copy;
	copy.create(output_size);

	for (size_t x = 0; x < copy.get_columns(); ++x) {
		for (size_t y = 0; y < copy.get_rows(); ++y) {
			copy.pixel({ x,y }) = source.pixel({ x + offset.x, y + offset.y });
		}
	}

	source = copy;
}