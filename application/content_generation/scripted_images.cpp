#include <experimental/filesystem>

#include "scripted_images.h"
#include "augs/gui/button_corners.h"
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"

#include "augs/templates/for_each_in_types.h"
#include "augs/misc/templated_readwrite.h"

#include "generated_introspectors.h"

namespace fs = std::experimental::filesystem;

void regenerate_scripted_images(
	const bool force_regenerate
) {
	const auto scripted_images_directory = "generated/scripted_images/";

	augs::create_directories(scripted_images_directory);

	const auto lines = augs::get_file_lines("scripted_images_generator_input.cfg");
	size_t current_line = 0;

	while (current_line < lines.size()) {
		scripted_image_stamp new_stamp;

		const auto target_stem = lines[current_line];
		
		ensure(target_stem.size() > 0);

		++current_line;

		while (current_line < lines.size() && lines[current_line].size() > 0) {
			std::istringstream in(lines[current_line]);

			std::string command_name;
			in >> command_name;

			for_each_type_in_list(augs::image::command_variant(), [&](auto dummy) {
				typedef decltype(dummy) command_type;

				if (command_name == command_type::get_command_name()) {
					command_type new_command;

					augs::read_members_from_istream(in, new_command);

					new_stamp.commands.push_back(new_command);
				}
			});

			++current_line;
		}

		// skip separating newline
		++current_line;

		const auto scripted_image_path = scripted_images_directory + target_stem + ".png";
		const auto scripted_image_stamp_path = scripted_images_directory + target_stem + ".stamp";

		augs::stream new_stamp_stream;

		augs::write(new_stamp_stream, new_stamp);

		bool should_regenerate = force_regenerate;
		
		if (!augs::file_exists(scripted_image_path)) {
			should_regenerate = true;
		}
		else {
			if (!augs::file_exists(scripted_image_stamp_path)) {
				should_regenerate = true;
			}
			else {
				augs::stream existent_stamp_stream;
				augs::assign_file_contents_binary(scripted_image_stamp_path, existent_stamp_stream);

				const bool are_stamps_identical = (new_stamp_stream == existent_stamp_stream);

				if (!are_stamps_identical) {
					should_regenerate = true;
				}
			}
		}

		if (should_regenerate) {
			LOG("Regenerating scripted image: %x", target_stem);

			augs::image resultant;

			for (const auto& c : new_stamp.commands) {
				resultant.execute(c);
			}

			resultant.save(scripted_image_path);

			augs::create_binary_file(scripted_image_stamp_path, new_stamp_stream);
		}
	}
}