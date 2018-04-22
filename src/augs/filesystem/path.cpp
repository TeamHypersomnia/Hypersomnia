#include "augs/filesystem/path.h"

std::string describe_moved_file(
	augs::path_type from, 
	augs::path_type to 
) {
	const auto f1 = from.filename();
	const auto f2 = to.filename();

	const auto d1 = augs::path_type(from).replace_filename("");
	const auto d2 = augs::path_type(to).replace_filename("");

	if (d1 == d2) {
		if (d1.empty()) {
			return typesafe_sprintf("%x to %x", f1, f2);
		}

		return typesafe_sprintf("%x to %x (%x)", f1, f2, d1);
	}

	return typesafe_sprintf("%x to %x (%x -> %x)", f1, f2, d1, d2);
}

