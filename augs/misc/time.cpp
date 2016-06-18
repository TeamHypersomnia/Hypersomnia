#include "time.h"
#include <ctime>
#include <sstream>

template<class T>
static std::string leading_zero(T component)
{
	std::stringstream out;
	out << component;
	std::string result = out.str();

	if (result.length() == 1)
		result = "0" + result;

	return result;
}

std::string augs::get_timestamp() {
	time_t current_time;
	time(&current_time);

	struct tm *local_time;
	local_time = localtime(&current_time);

	std::stringstream timestamp;

	return "[" 
		+ leading_zero(local_time->tm_mday) + "." 
		+ leading_zero(local_time->tm_mon + 1) + "." 
		+ leading_zero(local_time->tm_year + 1900) + "_" 
		+ leading_zero(local_time->tm_hour) + "." 
		+ leading_zero(local_time->tm_min) + "." 
		+ leading_zero(local_time->tm_sec) + 
		"]";
}