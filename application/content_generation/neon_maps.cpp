#include <sstream>
#include <experimental/filesystem>

#include "neon_maps.h"
#include "augs/filesystem/directory.h"
#include "augs/filesystem/file.h"

#include "augs/ensure.h"
#include "augs/misc/streams.h"

#include "augs/image/image.h"

namespace fs = std::experimental::filesystem;

void make_neon(
	const neon_map_stamp& stamp, 
	augs::image& source
);

void regenerate_neon_maps() {
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
		;

		const auto neon_map_path = neon_directory + source_path.filename().string();
		const auto neon_map_stamp_path = neon_directory + source_path.filename().replace_extension(".stamp").string();

		augs::stream new_stamp_stream;
		augs::write_object(new_stamp_stream, new_stamp);

		bool should_regenerate = false;

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

}