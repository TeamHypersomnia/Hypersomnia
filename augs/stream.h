#pragma once
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <iomanip>

namespace augs {
	template <typename T>
	void serialize(std::ofstream& f, const T& t) {
		f.write((const char*)&t, sizeof(T));
	}

	template <typename T>
	void deserialize(std::ifstream& f, T& t) {
		f.read((char*)&t, sizeof(T));
	}

	template <typename T>
	void serialize_vector(std::ofstream& f, const std::vector<T>& t) {
		size_t size = t.size();
		f.write((const char*)&size, sizeof(size));

		for (size_t i = 0; i < size; ++i)
			f.write((const char*)&t[i], sizeof(T));
	}

	template <typename T>
	void deserialize_vector(std::ifstream& f, std::vector<T>& t) {
		size_t size = 0;
		f.read((char*)&size, sizeof(size));

		for (size_t i = 0; i < size; ++i) {
			T obj;
			f.read((char*)&obj, sizeof(T));
			t.emplace_back(obj);
		}
	}

	/* number to string conversion */

	template <class T>
	std::string to_string(T val) {
		std::ostringstream ss;
		ss << val;
		return ss.str();
	}

	template <>
	std::string to_string(std::wstring val);

	/* number to wide string conversion */

	template <class T>
	std::wstring to_wstring(T val, int precision = -1, bool fixed = false) {
		std::wostringstream ss;

		if (precision > -1) {
			if (fixed) {
				ss << std::fixed;
			}

			ss << std::setprecision(precision);
		}

		ss << val;
		return ss.str();
	}

	std::wstring to_wstring(std::string val);

	template <class T>
	T to_value(std::wstring& s) {
		std::wistringstream ss(s);
		T v;
		ss >> v;
		return v;
	}

	template <class T>
	T to_value(std::string& s) {
		std::istringstream ss(s);
		T v;
		ss >> v;
		return v;
	}

	template <class A, class B>
	void clamp_string_value(std::wstring& s, A minval, B maxval) {
		if (s.empty()) s = to_wstring(minval);
		else {
			if (wnum<A>(s) < minval) s = to_wstring(minval);
			else if (wnum<B>(s) > maxval) s = to_wstring(maxval);
		}
	}

	template <class A, class B>
	void clamp_string_value(std::string& s, A minval, B maxval) {
		if (s.empty()) s = to_string(minval);
		else {
			if (wnum<A>(s) < minval) s = to_string(minval);
			else if (wnum<B>(s) > maxval) s = to_string(maxval);
		}
	}
}