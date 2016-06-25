#pragma once
#include <vector>
#include <type_traits>

template <typename B, typename T>
void serialize(B& buffer, const T& t) {
	// static_assert(std::is_pod<T>::value, "T must be a POD type.");
	buffer.write((const char*)&t, sizeof(T));
}

template <typename B, typename T>
void deserialize(B& buffer, T& t) {
	// static_assert(std::is_pod<T>::value, "T must be a POD type.");
	buffer.read((char*)&t, sizeof(T));
}

template <typename B, typename T>
void serialize(B& buffer, const std::vector<T>& vec) {
	serialize(buffer, vec.size());

	for (auto& val : vec)
		serialize(buffer, val);
}

template <typename B, typename T>
void deserialize(B& buffer, std::vector<T>& vec) {
	size_t size;
	deserialize(buffer, size);

	vec.resize(size);

	for (auto& val : vec)
		deserialize(buffer, val);
}