#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <unordered_map>

namespace augs {
	bool file_exists(std::string filename);
	std::string get_file_contents(std::string filename);

	std::vector<std::string> get_file_lines(const std::string& filename);

	template <class T>
	void write_file_binary(std::string filename, T& target) {
		std::ofstream out(filename, std::ios::out | std::ios::binary);
		out.write(target.data(), target.size());
	}

	template <class T>
	void assign_file_contents(const std::string& filename, T& target) {
		std::ifstream t(filename);

		t.seekg(0, std::ios::end);
		target.reserve(static_cast<unsigned>(t.tellg()));
		t.seekg(0, std::ios::beg);

		target.assign((std::istreambuf_iterator<char>(t)),
			std::istreambuf_iterator<char>());
	}

	template <class T>
	void assign_file_contents_binary(const std::string& filename, T& target) {
		std::ifstream file(filename, std::ios::binary | std::ios::ate);
		std::streamsize size = file.tellg();
		file.seekg(0, std::ios::beg);

		target.reserve(static_cast<unsigned>(size));
		file.read(target.data(), size);
	}

	template <class Key, class Value>
	void read_map_until_eof(const std::string& filename, std::unordered_map<Key, Value>& into) {
		std::ifstream source(filename, std::ios::in | std::ios::binary);

		while (source.peek() != EOF) {
			Key key;
			Value value;

			augs::read_object(source, key);
			augs::read_object(source, value);

			into.emplace(std::move(key), std::move(value));
		}
	}
}