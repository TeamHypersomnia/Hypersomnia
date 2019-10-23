#include "augs/log.h"
#include <sstream>

#include "augs/filesystem/directory.h"
#include "augs/filesystem/file.h"

#include "augs/ensure.h"
#include "augs/readwrite/memory_stream.h"
#include "augs/readwrite/to_bytes.h"

#include "augs/image/image.h"

#include "view/viewables/regeneration/desaturations.h"
#include "augs/readwrite/byte_file.h"

void regenerate_desaturation(
	const augs::path_type& source_path,
	const augs::path_type& output_path,
	const bool force_regenerate
) try {
	desaturation_stamp new_stamp;
	new_stamp.last_write_time_of_source = last_write_time(source_path);

	const auto desaturation_stamp_path = augs::path_type(output_path).replace_extension(".stamp");

	const auto new_stamp_bytes = augs::to_bytes(new_stamp);

	bool should_regenerate = force_regenerate;

	if (!augs::exists(output_path)) {
		should_regenerate = true;
	}
	else {
		if (!augs::exists(desaturation_stamp_path)) {
			should_regenerate = true;
		}
		else {
			const auto existent_stamp_bytes = augs::file_to_bytes(desaturation_stamp_path);
			const bool are_stamps_identical = (new_stamp_bytes == existent_stamp_bytes);

			if (!are_stamps_identical) {
				should_regenerate = true;
			}
		}
	}

	if (should_regenerate) {
		LOG("Regenerating desaturation for %x", source_path);

		augs::image input_image;
		input_image.from_file(source_path);
		input_image.desaturate().save(output_path);

		augs::create_directories_for(desaturation_stamp_path);
		augs::bytes_to_file(new_stamp_bytes, desaturation_stamp_path);
	}
}
catch (...) {
	augs::remove_file(output_path);
}