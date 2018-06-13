#include "augs/string/string_templates.h"
#include "augs/string/get_type_name.h"

std::string to_lowercase(std::string s) {
	return str_ops(s).to_lowercase().subject;
}

std::string to_uppercase(std::string s) {
	return str_ops(s).to_uppercase().subject;
}

std::string& uncapitalize_first(std::string& value) {
	if (value.size() > 0) {
		value[0] = ::tolower(value[0]);
	}

	return value;
}

std::string& capitalize_first(std::string& value) {
	if (value.size() > 0) {
		value[0] = ::toupper(value[0]);
	}

	return value;
}

std::string capitalize_first(std::string&& value) {
	return std::move(capitalize_first(value));
}

std::string uncapitalize_first(std::string&& value) {
	return std::move(uncapitalize_first(value));
}

std::string format_field_name(std::string s) {
	return str_ops(capitalize_first(s)).multi_replace_all({ "_", "." }, " ").subject;
}

std::string to_forward_slashes(std::string in_str) {
	for (auto& s : in_str) {
		if (s == '\\') {
			s = '/';
		}
	}

	return in_str;
}

bool begins_with(const std::string& value, const std::string& beginning) {
	if (beginning.size() > value.size()) {
		return false;
	}

	return std::equal(beginning.begin(), beginning.end(), value.begin());
}

bool ends_with(const std::string& value, const std::string& ending) {
	if (ending.size() > value.size()) {
		return false;
	}

	return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

std::string& cut_preffix(std::string& value, const std::string& preffix) {
	if (begins_with(value, preffix)) {
		value.erase(value.begin(), value.begin() + preffix.size());
	}

	return value;
}

std::string& cut_trailing(std::string& s, const char* const characters) {
	if (const auto it = s.find_last_not_of(characters); it != std::string::npos) {
		const auto len = s.size() - 1 - it;
		s.erase(s.end() - len, s.end());
	}

	return s;
}

std::string& cut_trailing_number(std::string& s) {
	return cut_trailing(s, "0123456789");
}

std::string& cut_trailing_spaces(std::string& s) {
	return cut_trailing(s, " ");
}

std::string& cut_trailing_number_and_spaces(std::string& s) {
	return cut_trailing(s, " 0123456789");
}

std::string cut_preffix(std::string&& value, const std::string& preffix) {
	return std::move(cut_preffix(value, preffix));
}

std::string cut_trailing(std::string&& s, const char* const characters) {
	return std::move(cut_trailing(s, characters));
}

std::string cut_trailing_number(std::string&& s) {
	return std::move(cut_trailing_number(s));
}

std::string cut_trailing_spaces(std::string&& s) {
	return std::move(cut_trailing_spaces(s));
}

std::string cut_trailing_number_and_spaces(std::string&& s) {
	return std::move(cut_trailing_number_and_spaces(s));
}

std::optional<unsigned long> get_trailing_number(const std::string& s) {
	try {
		return std::stoul(s.substr(s.find_last_not_of("0123456789") + 1));
	}
	catch (...) {

	}

	return std::nullopt;
}

#if BUILD_UNIT_TESTS
#include <catch.hpp>

namespace dummy_nmsp {
	struct dummy {

	};
}

TEST_CASE("Getting type's name") {
	REQUIRE("dummy_nmsp::dummy" == get_type_name<dummy_nmsp::dummy>());
	REQUIRE("dummy" == get_type_name_strip_namespace<dummy_nmsp::dummy>());
}

TEST_CASE("Templates StringTemplates") {
	{
		std::string test = "abababthis<abc> 8947208dj;;;abab</cdcde> isabab<cd>abcd</cd> the expected result";
		str_ops(test)
			.multi_replace_all({ "ab", "cd" }, "")
			.replace_all("<c> 8947208dj;;;</e>", "")
			.replace_all("<></>", "")
			;

		REQUIRE(test == "this is the expected result");
	}

	{
		std::string test = "abc";
		str_ops(test)
			.multi_replace_all({ "abc" }, "")
		;

		REQUIRE(test.empty());
	}

	{
		std::string test = "";
		str_ops(test)
			.multi_replace_all({ " " }, "")
		;

		REQUIRE(test.empty());
	}
}
#endif
