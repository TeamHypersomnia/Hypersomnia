#include <sstream>

#include "desaturations.h"
#include "augs/filesystem/directory.h"
#include "augs/filesystem/file.h"

#include "augs/ensure.h"
#include "augs/misc/streams.h"

#include "augs/image/image.h"

void regenerate_desaturations(
	const bool force_regenerate
) {
	const auto desaturations_directory = "generated/desaturations/";

	augs::create_directories(desaturations_directory);

	const auto lines = augs::get_file_lines("cfg/desaturations_generator_input.cfg");
	size_t current_line = 0;

	while (current_line < lines.size()) {
		desaturation_stamp new_stamp;

		const auto source_path = lines[current_line];

		new_stamp.last_write_time_of_source = augs::last_write_time(source_path);

		const auto desaturation_path = desaturations_directory + augs::get_filename(source_path);
		const auto desaturation_stamp_path = desaturations_directory + augs::replace_extension(augs::get_filename(source_path), ".stamp");

		augs::stream new_stamp_stream;
		augs::write(new_stamp_stream, new_stamp);

		bool should_regenerate = force_regenerate;

		if (!augs::file_exists(desaturation_path)) {
			should_regenerate = true;
		}
		else {
			if (!augs::file_exists(desaturation_stamp_path)) {
				should_regenerate = true;
			}
			else {
				augs::stream existent_stamp_stream;
				augs::get_file_contents_binary_into(desaturation_stamp_path, existent_stamp_stream);

				const bool are_stamps_identical = (new_stamp_stream == existent_stamp_stream);

				if (!are_stamps_identical) {
					should_regenerate = true;
				}
			}
		}

		if (should_regenerate) {
			LOG("Regenerating desaturation for %x", source_path);

			augs::image source_image;
			source_image.from_file(source_path);
			source_image.get_desaturated().save(desaturation_path);

			augs::create_binary_file(desaturation_stamp_path, new_stamp_stream);
		}

		++current_line;
	}
}