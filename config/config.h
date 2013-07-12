#pragma once
#include <unordered_map>
#include <string>
#include <functional>
#include <fstream>
#include <tuple>

namespace augmentations {
	namespace config {
		struct input_file {
			typedef std::tuple<unsigned char, unsigned char, unsigned char, unsigned char> color;
			struct property {
				std::wstring _type;
				std::wstring string_val;
				int int_val;
				float float_val;
				double double_val;
				operator int() const;
				operator float() const;
				operator double() const;
				operator std::wstring() const;
				operator color() const;
			};

			std::unordered_map<std::wstring, property> values;

			/* key is property's type, value is pair of stream handlers, stream will be set exactly before value */
			std::unordered_map<std::wstring, 
				std::pair<std::function<void(std::wifstream&, property&)>, std::function<void(std::wofstream&, property&)>>> callbacks;

			input_file();
			input_file(const char* filename);
			void open(const char* filename);
			void save(const char* output_filename);

			property& operator[](std::wstring name);
		};
	}
}