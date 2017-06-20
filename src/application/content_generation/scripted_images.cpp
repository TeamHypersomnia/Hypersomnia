#include "scripted_images.h"
#include "augs/gui/button_corners.h"
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "augs/misc/streams.h"
#include "augs/templates/for_each_in_types.h"
#include "augs/misc/templated_readwrite.h"

void regenerate_scripted_image(
	const std::string& output_image_path,
	const scripted_image_input& input,
	const bool force_regenerate
) {
	const auto output_image_stamp_path = augs::replace_extension(output_image_path, ".stamp");

	augs::stream new_stamp_stream;
	augs::write(new_stamp_stream, input);

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

		augs::create_binary_file(output_image_stamp_path, new_stamp_stream);
	}
}