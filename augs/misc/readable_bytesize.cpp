#include "readable_bytesize.h"

std::string readable_bytesize(unsigned _size) {
	double size = static_cast<double>(_size);

	int i = 0;
	const char* units[] = { "B", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB" };
	while (size > 1024) {
		size /= 1024;
		i++;
	}

	char buf[100];
	std::fill(buf, buf + 100, 0);

	sprintf(buf, "%.*f %s", i, size, units[i]);
	return std::string(buf);
}