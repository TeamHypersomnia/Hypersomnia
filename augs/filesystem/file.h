#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <chrono>

namespace augs {
	class stream;

	std::chrono::system_clock::time_point last_write_time(const std::string& path);

	void ensure_existence(const std::string& path);

	bool file_exists(const std::string& path);
	std::string get_file_contents(const std::string& path);

	std::vector<std::string> get_file_lines(const std::string& path);

	template <class T>
	void create_text_file(const T& path, const T& text) {
		std::ofstream out(path, std::ios::out);
		out << text;
	}

	template <class T>
	void create_binary_file(std::string path, T& target) {
		std::ofstream out(path, std::ios::out | std::ios::binary);
		out.write(target.data(), target.size());
	}

	template <class T>
	void assign_file_contents(const std::string& path, T& target) {
		std::ifstream t(path);

		t.seekg(0, std::ios::end);
		target.reserve(static_cast<unsigned>(t.tellg()));
		t.seekg(0, std::ios::beg);

		target.assign((std::istreambuf_iterator<char>(t)),
			std::istreambuf_iterator<char>());
	}

	void assign_file_contents_binary(const std::string& path, augs::stream& target);

	template <class ContainerType>
	void read_map_until_eof(const std::string& path, ContainerType& into) {
		std::ifstream source(path, std::ios::in | std::ios::binary);

		while (source.peek() != EOF) {
			typename ContainerType::key_type key;
			typename ContainerType::mapped_type value;

			augs::read_object(source, key);
			augs::read_object(source, value);

			into.emplace(std::move(key), std::move(value));
		}
	}
}