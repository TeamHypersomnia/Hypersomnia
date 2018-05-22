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

std::string&& uncapitalize_first(std::string&& value) {
	uncapitalize_first(value);
	return std::move(value);
}

std::string& capitalize_first(std::string& value) {
	if (value.size() > 0) {
		value[0] = ::toupper(value[0]);
	}

	return value;
}

std::string&& capitalize_first(std::string&& value) {
	capitalize_first(value);
	return std::move(value);
}

std::string format_field_name(std::string s) {
	s[0] = ::toupper(s[0]);
	return str_ops(s).multi_replace_all({ "_", "." }, " ").subject;
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
