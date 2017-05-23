#include "augs/templates/string_templates.h"

std::wstring to_wstring(const std::string& val) {
	return std::wstring(val.begin(), val.end());
}

std::string to_string(const std::wstring& val) {
	return std::string(val.begin(), val.end());
}
