#pragma once
#include <array>
#include <string>

namespace augs {
	template <unsigned const_count>
	class constant_size_string {
		using array_type = std::array<char, const_count + 1>;

		unsigned len = 0;
		array_type arr = {};

	public:
		constant_size_string() = default;

		constant_size_string(const std::string& ss) {
			len = std::min(std::size_t(const_count), ss.size());

			std::memcpy(arr.data(), ss.data(), len);
			arr[len] = 0;
		}

		constant_size_string(const char* s) {
			auto it = arr.begin();

			while (it != arr.end() - 1 && *s) {
				*it++ = *s++;
			}

			*it = 0;
			len = it - arr.begin();
		}

		operator std::string() const {
			return arr.data();
		}

		bool operator==(const std::string& b) const {
			return len == b.length() && !std::memcmp(data(), b.data(), len);
		}

		template <unsigned B>
		bool operator==(const constant_size_string<B>& b) const {
			return len == b.len && !std::memcmp(data(), b.data(), len);
		}

		template <unsigned B>
		bool operator!=(const constant_size_string<B>& b) const {
			return !operator==(b);
		}

		template <unsigned B>
		bool operator<(const constant_size_string<B>& b) const {
			return std::strcmp(c_str(), b.c_str()) < 0;
		}

		template <unsigned B>
		bool operator>(const constant_size_string<B>& b) const {
			return std::strcmp(c_str(), b.c_str()) > 0;
		}

		const auto* c_str() const {
			return arr.data();
		}

		auto* data() {
			return arr.data();
		}

		auto* data() const {
			return arr.data();
		}

		auto size() const {
			return std::size_t(len);
		}

		auto length() const {
			return std::size_t(len);
		}

		bool empty() const {
			return size() == 0;
		}

		auto begin() {
			return arr.begin();
		}

		auto end() {
			return arr.begin() + size();
		}

		auto begin() const {
			return arr.begin();
		}

		auto end() const {
			return arr.begin() + size();
		}

		constexpr std::size_t max_size() const noexcept {
			return const_count;
		}

		void resize_no_init(const std::size_t n) {
			len = n;
			arr[len] = 0;
		}

		auto& operator[](const std::size_t n) {
			return arr[n];
		}

		const auto& operator[](const std::size_t n) const {
			return arr[n];
		}
		
		void clear() {
			len = 0;
			arr[0] = 0;
		}
	};
}

template <unsigned const_count>
std::ostream& operator<<(std::ostream& out, const augs::constant_size_string<const_count>& x) {
	return out << x.c_str();
}
