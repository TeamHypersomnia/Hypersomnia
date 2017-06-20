#include <sstream>
#include <experimental/filesystem>

#include "polygonizations_of_images.h"

#include "augs/filesystem/directory.h"
#include "augs/filesystem/file.h"

#include "augs/ensure.h"
#include "augs/misc/streams.h"

#include "augs/image/image.h"
#include "generated/introspectors.h"

void regenerate_polygonization_of_image(
	const std::string& source_red_black_image,
	const std::string& output_path,
	const bool force_regenerate
) {
	polygonization_of_image_stamp new_stamp;
	new_stamp.last_write_time_of_source = augs::last_write_time(source_red_black_image);

	const auto polygonization_stamp_path = augs::replace_extension(output_path, ".stamp");

	augs::stream new_stamp_stream;
	augs::write(new_stamp_stream, new_stamp);

	bool should_regenerate = force_regenerate;

	if (!augs::file_exists(output_path)) {
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
		LOG("Regenerating polygonization for %x", source_red_black_image);

		augs::image source_image;
		source_image.from_file(source_red_black_image);

		const auto points = source_image.get_polygonized();

		std::ostringstream in;

		for (const auto& p : points) {
			in << p << std::endl;
		}

		augs::create_text_file(output_path, in.str());

		augs::create_binary_file(polygonization_stamp_path, new_stamp_stream);
	}
}