#include "string_templates.h"

std::string to_lowercase(std::string s) {
	return str_ops(s).to_lowercase().subject;
}

std::string to_uppercase(std::string s) {
	return str_ops(s).to_uppercase().subject;
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

#if PLATFORM_UNIX
#include <cstdlib>
#include <memory>
#include <cxxabi.h>

std::string demangle(const char* name) {
    int status = -4; // some arbitrary value to eliminate the compiler warning
	
    // enable c++11 by passing the flag -std=c++11 to g++
    std::unique_ptr<char, void(*)(void*)> res {
        abi::__cxa_demangle(name, NULL, NULL, &status),
        std::free
    };

    return (status==0) ? res.get() : name ;
}

#else

// does nothing if not g++
std::string demangle(const char* name) {
    return name;
}

#endif
