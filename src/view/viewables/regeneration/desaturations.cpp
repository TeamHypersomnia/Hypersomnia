#include <sstream>

#include "augs/filesystem/directory.h"
#include "augs/filesystem/file.h"

#include "augs/ensure.h"
#include "augs/readwrite/streams.h"

#include "augs/image/image.h"

#include "view/viewables/regeneration/desaturations.h"
#include "augs/readwrite/byte_readwrite.h"

void regenerate_desaturation(
	const augs::path_type& source_path,
	const augs::path_type& output_path,
	const bool force_regenerate
) {
	desaturation_stamp new_stamp;
	new_stamp.last_write_time_of_source = last_write_time(source_path);

	const auto desaturation_stamp_path = augs::path_type(output_path).replace_extension(".stamp");

	augs::stream new_stamp_stream;
	augs::write(new_stamp_stream, new_stamp);

	bool should_regenerate = force_regenerate;

	if (!augs::file_exists(output_path)) {
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

		augs::image(source_path).get_desaturated().save(output_path);

		augs::create_directories(desaturation_stamp_path);
		augs::create_binary_file(desaturation_stamp_path, new_stamp_stream);
	}
}