#include <experimental/filesystem>

#include "scripted_images.h"
#include "augs/gui/button_corners.h"
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"

#include "augs/templates/for_each_in_types.h"
#include "augs/misc/introspect.h"

namespace fs = std::experimental::filesystem;

void regenerate_scripted_images() {
	const auto scripted_images_directory = "generated/scripted_images/";

	augs::create_directories(scripted_images_directory);

	const auto lines = augs::get_file_lines("scripted_images_generator_input.cfg");
	size_t current_line = 0;

	while (current_line < lines.size()) {
		scripted_image_metadata new_meta;

		const auto target_stem = lines[current_line];
		++current_line;

		while (lines[current_line] != "\n") {
			std::istringstream in(lines[current_line]);

			std::string command_name;
			in >> command_name;
		
			for_each_type_in_list(augs::image::command_variant(), [&](auto dummy) {
				typedef decltype(dummy) command_type;

				if (command_name == command_type::get_command_name()) {
					command_type new_command;

					augs::introspect(new_command, [&](auto& member) {
						in >> member;
					});

					new_meta.commands.push_back(new_command);
				}
			});
		}

		const auto scripted_image_filename = scripted_images_directory + target_stem + ".png";
		const auto scripted_image_meta_filename = scripted_images_directory + target_stem + ".meta";

		augs::stream new_meta_stream;
		augs::write_object(new_meta_stream, new_meta);

		bool should_regenerate = false;
		
		if (!augs::file_exists(scripted_image_filename)) {
			should_regenerate = true;
		}
		else {
			if (!augs::file_exists(scripted_image_meta_filename)) {
				should_regenerate = true;
			}
			else {
				augs::stream existent_meta_stream;
				augs::assign_file_contents_binary(scripted_image_meta_filename, existent_meta_stream);

				const bool are_metas_identical = (new_meta_stream == existent_meta_stream);

				if (!are_metas_identical) {
					should_regenerate = true;
				}
			}
		}

		if (should_regenerate) {
			LOG("Regenerating scripted image: %x", target_stem);

			augs::image resultant;

			for (const auto& c : new_meta.commands) {
				resultant.execute(c);
			}

			resultant.save(scripted_image_filename);

			augs::create_binary_file(scripted_image_meta_filename, new_meta_stream);
		}

		// skip separating newline
		++current_line;
	}
}