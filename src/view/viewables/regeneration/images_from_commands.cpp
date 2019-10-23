#include "augs/log.h"
#include "augs/gui/button_corners.h"
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "augs/readwrite/memory_stream.h"
#include "augs/readwrite/to_bytes.h"
#include "augs/templates/for_each_std_get.h"

#include "view/viewables/regeneration/images_from_commands.h"

#include "augs/readwrite/byte_file.h"

void regenerate_image_from_commands(
	const augs::path_type& output_image_path,
	const image_from_commands_input& input,
	const bool force_regenerate
) {
	const auto output_image_stamp_path = augs::path_type(output_image_path).replace_extension(".stamp");

	const auto new_stamp_bytes = augs::to_bytes(input);

	bool should_regenerate = force_regenerate;

	if (!augs::exists(output_image_path)) {
		should_regenerate = true;
	}
	else {
		if (!augs::exists(output_image_stamp_path)) {
			should_regenerate = true;
		}
		else {
			const auto existent_stamp_bytes = augs::file_to_bytes(output_image_stamp_path);
			const bool are_stamps_identical = (new_stamp_bytes == existent_stamp_bytes);

			if (!are_stamps_identical) {
				should_regenerate = true;
			}
		}
	}

	if (should_regenerate) {
		LOG("Regenerating scripted image: %x", output_image_path);

		augs::image resultant;

		for (const auto& c : input.commands) {
			resultant.execute(c);
		}

		resultant.save(output_image_path);

		augs::create_directories_for(output_image_stamp_path);
		augs::bytes_to_file(new_stamp_bytes, output_image_stamp_path);
	}
}