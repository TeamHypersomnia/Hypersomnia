#include "augs/gui/button_corners.h"
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "augs/readwrite/streams.h"
#include "augs/templates/for_each_std_get.h"

#include "view/viewables/regeneration/images_from_commands.h"

#include "augs/readwrite/byte_readwrite.h"

void regenerate_image_from_commands(
	const augs::path_type& output_image_path,
	const image_from_commands_input& input,
	const bool force_regenerate
) {
	const auto output_image_stamp_path = augs::path_type(output_image_path).replace_extension(".stamp");

	augs::stream new_stamp_stream;
	augs::write_bytes(new_stamp_stream, input);

	bool should_regenerate = force_regenerate;

	if (!augs::file_exists(output_image_path)) {
		should_regenerate = true;
	}
	else {
		if (!augs::file_exists(output_image_stamp_path)) {
			should_regenerate = true;
		}
		else {
			augs::stream existent_stamp_stream;
			augs::get_file_contents_binary_into(output_image_stamp_path, existent_stamp_stream);

			const bool are_stamps_identical = (new_stamp_stream == existent_stamp_stream);

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

		augs::create_directories(output_image_stamp_path);
		augs::create_binary_file(output_image_stamp_path, new_stamp_stream);
	}
}