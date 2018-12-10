#pragma once
#include <array>
#include <string>

namespace augs {
	template <unsigned const_count>
	class constant_size_string {
		using array_type = std::array<char, const_count>;

		array_type data;

	public:
		constant_size_string() {
			data[0] = 0;
		}

		constant_size_string(const std::string& ss) {
			const auto s = std::min(std::size_t(const_count - 1), ss.size());

			std::memcpy(data.data(), ss.data(), s);
			data[s] = 0;
		}

		constant_size_string(const char* s) {
			data[0] = 0;

			auto it = data.begin();

			while (it != data.end() - 1 && *s) {
				*it = *s++;
			}

			*it = 0;
		}

		operator std::string() const {
			return data.data();
		}
	};
}