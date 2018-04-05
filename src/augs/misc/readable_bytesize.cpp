#include "readable_bytesize.h"
#include "augs/misc/typesafe_sprintf.h"

std::string readable_bytesize(unsigned _size) {
	double size = static_cast<double>(_size);

	int i = 0;
	const char* units[] = { "B", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB" };
	while (size > 1024) {
		size /= 1024;
		i++;
	}

	return typesafe_sprintf("%x %x", size, units[i]);
}