#pragma once
#include <string>
#include <sstream>
#include <utility>
#include <type_traits>
#include <limits>

#include "augs/ensure.h"
#include "augs/templates/string_templates.h"

template <typename CharType>
bool typesafe_scanf_detail(
	size_t source_pos,
	size_t format_pos,
	const std::basic_string<CharType>& source_string,
	const std::basic_string<CharType>& format
) {
	return true;
}

template<typename CharType, typename T, typename... A>
bool typesafe_scanf_detail(
	size_t source_pos,
	size_t format_pos,
	const std::basic_string<CharType>& source_string,
	const std::basic_string<CharType>& format,
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

			if (found_terminating == std::string::npos) {
				return false;
			}
			else {
				std::basic_istringstream<CharType> read_chunk(
					source_string.substr(
						value_beginning_at_source, 
						found_terminating - value_beginning_at_source
					)
				);

				read_chunk >> val;

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
		}
		else {
			std::basic_istringstream<CharType> read_chunk(
				source_string.substr(
					value_beginning_at_source
				)
			);

			if (sizeof...(A) == 0) {
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

template<typename CharType, typename... A>
bool typesafe_sscanf(
	const std::basic_string<CharType>& source_string,
	const std::basic_string<CharType>& format,
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

template<typename C1, typename C2, typename... A>
bool typesafe_sscanf(
	const C1 source_string,
	const C2 format,
	A&... a
) {
	return typesafe_scanf_detail(
		0, 
		0, 
		std::basic_string<get_underlying_char_type_t<C1>>(source_string),
		std::basic_string<get_underlying_char_type_t<C2>>(format),
		a...
	);
}