#include "augs/string/get_type_name.h"
#include "augs/string/string_templates.h"

static auto remove_preffix(std::string name) {
	return str_ops(name).multi_replace_all({ "struct ", "class ", "enum " }, "").subject;
}

#if PLATFORM_UNIX
#include <cstdlib>
#include <memory>
#include <cxxabi.h>

std::string demangle(const char* name) {
	int status = -4; // some arbitrary value to eliminate the compiler warning

	std::unique_ptr<char, void(*)(void*)> res {
		abi::__cxa_demangle(name, NULL, NULL, &status),
		std::free
	};

	return remove_preffix((status==0) ? res.get() : name);
}

#else

// does nothing if not g++
std::string demangle(const char* name) {
	return remove_preffix(name);
}

#endif
