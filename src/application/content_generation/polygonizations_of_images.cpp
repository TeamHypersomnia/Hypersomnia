#include <sstream>
#include <experimental/filesystem>

#include "polygonizations_of_images.h"

#include "augs/filesystem/directory.h"
#include "augs/filesystem/file.h"

#include "augs/ensure.h"
#include "augs/misc/streams.h"

#include "augs/image/image.h"
#include "generated_introspectors.h"

void regenerate_polygonizations_of_images(
	const bool force_regenerate
) {
	const auto polygonizations_directory = "generated/polygonizations_of_images/";

	augs::create_directories(polygonizations_directory);

	const auto lines = augs::get_file_lines("cfg/polygonizations_of_images_generator_input.cfg");
	size_t current_line = 0;

	while (current_line < lines.size()) {
		polygonization_of_image_stamp new_stamp;

		const auto source_path = lines[current_line];
		new_stamp.last_write_time_of_source = augs::last_write_time(source_path);

		++current_line;
		
		const auto target_filename = lines[current_line];
		
		++current_line;

		// skip newline
		++current_line;

		const auto polygonization_target_path = polygonizations_directory + target_filename;
		const auto polygonization_stamp_path = augs::replace_extension(polygonization_target_path, ".stamp");

		augs::stream new_stamp_stream;
		augs::write(new_stamp_stream, new_stamp);

		bool should_regenerate = force_regenerate;

		if (!augs::file_exists(polygonization_target_path)) {
			should_regenerate = true;
		}
		else {
			if (!augs::file_exists(polygonization_stamp_path)) {
				should_regenerate = true;
			}
			else {
				augs::stream existent_stamp_stream;
				augs::get_file_contents_binary_into(polygonization_stamp_path, existent_stamp_stream);

				const bool are_stamps_identical = (new_stamp_stream == existent_stamp_stream);

				if (!are_stamps_identical) {
					should_regenerate = true;
				}
			}
		}

		if (should_regenerate) {
			LOG("Regenerating polygonization for %x", source_path);

			augs::image source_image;
			source_image.from_file(source_path);
			
			const auto points = source_image.get_polygonized();
			
			std::ostringstream in;

			for (const auto& p : points) {
				in << p << std::endl;
			}

			augs::create_text_file(polygonization_target_path, in.str());

			augs::create_binary_file(polygonization_stamp_path, new_stamp_stream);
		}
	}
}