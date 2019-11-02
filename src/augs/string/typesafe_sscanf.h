#pragma once
#include <string>
#include <sstream>
#include <utility>
#include <type_traits>
#include <limits>

#include "augs/log.h"
inline int typesafe_scanf_detail(
	size_t,
	size_t,
	const std::string&,
	const std::string&
) {
	return 0;
}

template <typename T>
int detail_typesafe_scanf_value(
	std::istringstream& read_chunk,
	T& into
) {
	T cp;

	if (read_chunk >> cp) {
		into = cp;
		return 1;
	}

	return 0;
}

inline int detail_typesafe_scanf_value(
	std::istringstream& read_chunk,
	std::string& into
) {
	into = read_chunk.str();
	return 1;
}

template <typename T, typename... A>
int typesafe_scanf_detail(
	size_t source_pos,
	size_t format_pos,
	const std::string& source_string,
	const std::string& format,
	T& val,
	A&... vals
) {
	if (source_pos == std::string::npos) {
		return 0;
	}

	const auto previous_format_pos = format_pos;

	format_pos = format.find("%x", format_pos);

	const auto how_many_format_advanced = format_pos - previous_format_pos;

	if (format_pos != std::string::npos) {
		const auto value_beginning_at_source = source_pos + how_many_format_advanced;

		if (value_beginning_at_source < source_string.size() && format_pos + 2 <= format.size()) {
			const auto terminating_character = format[format_pos + 2];
			const auto found_terminating = source_string.find(terminating_character, value_beginning_at_source);

			int read_elements = 0;

			{
				/* if found_terminating is std::string::npos, we take the rest of the string */

				const auto read_substr = source_string.substr(
					value_beginning_at_source, 
					found_terminating - value_beginning_at_source
				);

				auto read_chunk = std::istringstream(read_substr);

				read_elements = detail_typesafe_scanf_value(read_chunk, val);
			}

			source_pos = found_terminating;
			format_pos = format_pos + 2;

			return read_elements + typesafe_scanf_detail(
				source_pos,
				format_pos,
				source_string,
				format,
				vals...
			);
		}
	}

	return 0;
}

template <typename... A>
bool typesafe_sscanf(
	const std::string& source_string,
	const std::string& format,
	A&... a
) {
	return sizeof...(A) == typesafe_scanf_detail(
		0, 
		0, 
		source_string,
		format,
		a...
	);
}