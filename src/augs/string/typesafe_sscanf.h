#pragma once
#include <string>
#include <sstream>
#include <utility>
#include <type_traits>
#include <limits>

inline bool typesafe_scanf_detail(
	size_t,
	size_t,
	const std::string&,
	const std::string&
) {
	return true;
}

template <typename T>
void detail_typesafe_scanf_value(
	std::istringstream& read_chunk,
	T& into
) {
	read_chunk >> into;
}

inline void detail_typesafe_scanf_value(
	std::istringstream& read_chunk,
	std::string& into
) {
	into = read_chunk.str();
}

template <typename T, typename... A>
bool typesafe_scanf_detail(
	size_t source_pos,
	size_t format_pos,
	const std::string& source_string,
	const std::string& format,
	T& val,
	A&... vals
) {
	const auto previous_format_pos = format_pos;

	format_pos = format.find('%', format_pos);

	const auto how_many_format_advanced = format_pos - previous_format_pos;

	if (format_pos != std::string::npos) {
		const auto value_beginning_at_source = source_pos + how_many_format_advanced;

		if (format_pos + 2 < format.size()) {
			const auto terminating_character = format[format_pos + 2];
			const auto found_terminating = source_string.find(terminating_character, value_beginning_at_source);

			{
				/* if found_terminating is std::string::npos, we take the rest of the string */

				std::istringstream read_chunk(
					source_string.substr(
						value_beginning_at_source, 
						found_terminating - value_beginning_at_source
					)
				);

				detail_typesafe_scanf_value(read_chunk, val);
			}

			source_pos = found_terminating;
			format_pos = format_pos + 2;

			return typesafe_scanf_detail(
				source_pos,
				format_pos,
				source_string,
				format,
				vals...
			);
		}
		else {
			{
				std::istringstream read_chunk(
					source_string.substr(
						value_beginning_at_source
					)
				);

				detail_typesafe_scanf_value(read_chunk, val);
			}

			if constexpr(sizeof...(A) == 0) {
				return true;
			}
			else {
				return false;
			}
		}
	}
	else {
		return false;
	}
}

template <typename... A>
bool typesafe_sscanf(
	const std::string& source_string,
	const std::string& format,
	A&... a
) {
	return typesafe_scanf_detail(
		0, 
		0, 
		source_string,
		format,
		a...
	);
}