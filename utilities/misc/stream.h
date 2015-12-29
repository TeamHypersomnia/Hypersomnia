#pragma once
#include <string>
#include <sstream>
#include <fstream>
#include <vector>

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

		for (auto& p : t)
			f.write((const char*)&p, sizeof(p));
	}

	template <typename T>
	void deserialize_vector(std::ifstream& f, std::vector<T>& t) {
		size_t size;
		f.read((char*)&size, sizeof(size));

		t.resize(size);

		for (auto& p : t)
			f.read((char*)&p, sizeof(p));
	}

	namespace misc {

		/* number to string conversion */

		template <class T>
		std::string str(T val) {
			std::ostringstream ss;
			ss << val;
			return ss.str();
		}
		
		/* number to wide string conversion */

		template <class T>
		std::wstring wstr(T val) {
			std::wostringstream ss;
			ss << val;
			return ss.str();
		}
		
		/* wide string to number conversion */

		template <class T>
		T wnum(std::wstring& s) {
			std::wistringstream ss(s);
			T v;
			ss >> v;
			return v;
		}
		
		/* string to number conversion */

		template <class T>
		T num(std::string& s) {
			std::istringstream ss(s);
			T v;
			ss >> v;
			return v;
		}
		
		template <class A, class B>
		void bound(std::wstring& s, A minval, B maxval) {
			if(s.empty()) s = wstr(minval);
			else {
				if(wnum<A>(s) < minval) s = wstr(minval);
				else if(wnum<B>(s) > maxval) s = wstr(maxval);
			}
		}
	}
}