#pragma once

template <class T>
auto to_forward_slashes(T str) {
	std::replace(str.begin(), str.end(), '\\', '/');
	return str;
}


