#include "readable_bytesize.h"
#include "augs/string/typesafe_sprintf.h"

std::string readable_bytesize(const std::size_t _size, const char* number_format) {
	double size = static_cast<double>(_size);

	int i = 0;
	const char* units[] = { "B", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB" };
	while (size > 1000) {
		size /= 1000;
		i++;
	}

	const auto number = typesafe_sprintf(number_format, size);

	return typesafe_sprintf("%x %x", number, units[i]);
}

std::string readable_bitsize(const std::size_t _size, const char* number_format) {
	double size = static_cast<double>(_size);

	int i = 0;
	const char* units[] = { "bit", "kbit", "Mbit", "Gbit", "Tbit", "Pbit", "Ebit", "Zbit", "Ybit" };

	while (size > 1000) {
		size /= 1000;
		i++;
	}

	const auto number = typesafe_sprintf(number_format, size);

	return typesafe_sprintf("%x %x", number, units[i]);
}