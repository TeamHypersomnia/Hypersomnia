#pragma once
#include <string>

std::string readable_bytesize(std::size_t bytes, const char* number_format = "%x");
std::string readable_bitsize(std::size_t bytes, const char* number_format = "%x");