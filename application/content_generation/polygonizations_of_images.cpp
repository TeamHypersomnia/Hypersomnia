#include <sstream>
#include <experimental/filesystem>

#include "polygonizations_of_images.h"

#include "augs/filesystem/directory.h"
#include "augs/filesystem/file.h"

#include "augs/ensure.h"
#include "augs/misc/streams.h"

#include "augs/image/image.h"

namespace fs = std::experimental::filesystem;

void regenerate_polygonizations_of_images() {
	const auto polygonizations_directory = "generated/polygonizations_of_images/";

	augs::create_directories(polygonizations_directory);

	const auto lines = augs::get_file_lines("polygonizations_of_images_generator_input.cfg");
	size_t current_line = 0;

	while (current_line < lines.size()) {
		polygonization_of_image_metadata new_meta;

		const auto source_path = fs::path(lines[current_line]);
		new_meta.last_write_time_of_source = fs::last_write_time(source_path);

		++current_line;
		
		const auto target_filename = lines[current_line];
		
		++current_line;

		// skip newline
		++current_line;

		const auto polygonization_target_path = polygonizations_directory + target_filename;
		const auto polygonization_meta_path = fs::path(polygonization_target_path).replace_extension(".meta").string();

		augs::stream new_meta_stream;
		augs::write_object(new_meta_stream, new_meta);

		bool should_regenerate = false;

		if (!augs::file_exists(polygonization_target_path)) {
			should_regenerate = true;
		}
		else {
			if (!augs::file_exists(polygonization_meta_path)) {
				should_regenerate = true;
			}
			else {
				augs::stream existent_meta_stream;
				augs::assign_file_contents_binary(polygonization_meta_path, existent_meta_stream);

				const bool are_metas_identical = (new_meta_stream == existent_meta_stream);

				if (!are_metas_identical) {
					should_regenerate = true;
				}
			}
		}

		if (should_regenerate) {
			LOG("Regenerating polygonization for %x", source_path.string());

			augs::image source_image;
			source_image.from_file(source_path.string());
			
			const auto points = source_image.get_polygonized();
			
			std::ostringstream in;

			for (const auto& p : points) {
				in << p.x << " " << p.y << std::endl;
			}

			augs::create_text_file(polygonization_target_path, in.str());

			augs::create_binary_file(polygonization_meta_path, new_meta_stream);
		}
	}
}